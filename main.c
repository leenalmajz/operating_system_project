#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_PROCESSES 128
#define MAX_ARGS 128
pid_t background_processes[MAX_PROCESSES]; // Array to track background PIDs
int bg_process_count = 0;

extern int handle_redirect(char *cmd, char *filename, int background);

void check_background_processes() {
    for (int i = 0; i < bg_process_count; i++) {
        if (background_processes[i] != -1) {
            int status;
            pid_t result = waitpid(background_processes[i], &status, WNOHANG);
            if (result > 0) { // Process has finished
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    background_processes[i] = -1; // Remove the process from the array
                }
            }
        }
    }

    // Compact the array to remove -1 entries
    int new_count = 0;
    for (int i = 0; i < bg_process_count; i++) {
        if (background_processes[i] != -1) {
            background_processes[new_count++] = background_processes[i];
        }
    }
    bg_process_count = new_count;
}

int handle_exec(char *cmd) {
    cmd += 5; // Ignore "exec "
    char *args[MAX_ARGS];
    int background = 0;
    char *output_file = NULL;

    // Handle output redirection first
    char *redirect = strchr(cmd, '>');
    if (redirect) {
        *redirect = '\0'; // Terminate the command string at '>'
        redirect++;
        while (*redirect == ' ') redirect++; // Skip spaces after '>'
        if (*redirect == '\0') {
            perror("exec failed: No output file specified");
            return 1;
        }
        output_file = strdup(redirect);
        if (!output_file) {
            perror("strdup failed");
            return 1;
        }
    }

    // Handle background execution '&' at the end of the command
    char *bg_flag = strrchr(cmd, '&');
    if (bg_flag && *(bg_flag + 1) == '\0') { // Ensure '&' is at the end of the command
        *bg_flag = '\0';
        background = 1;
    }

    // Tokenize the command string
    char *token = strtok(cmd, " ");
    int count = 0;
    while (token != NULL && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = strtok(NULL, " ");
    }
    args[count] = NULL; // Null-terminate the arguments

    if (count == 0) return 1; // No command provided

    // Handle the redirection
    if (output_file) {
        handle_redirect(cmd, output_file, background);
        free(output_file);
        return 0;
    }

    // Execute the command
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) { // Child process
        execvp(args[0], args);
        perror("exec failed"); // Only runs if exec fails
        exit(1);
    } else { // Parent process
        if (!background) {
            waitpid(pid, NULL, 0); // Wait for child to finish
        } else {
            if (bg_process_count < MAX_PROCESSES) {
                background_processes[bg_process_count++] = pid; // Add PID to the array
                printf("Background process started with PID: %d\n", pid);
            } else {
                printf("Maximum background process limit reached.\n");
            }
        }
    }
    return 0;
}

int handle_globalusage(char *cmd) {
    char *output_file = NULL;

    char *redirect = strchr(cmd, '>');
    if (redirect) {
        *redirect = '\0';
        redirect++;
        while (*redirect == ' ') redirect++;  // skipping spaces until reaching the fie name
        if (*redirect == '\0') {
            perror("exec failed: No output file specified");
            return 1;
        }
        output_file = strdup(redirect);
        if(!output_file) {
            perror("strdup failed");
            return 1;
        }
    }

    if (output_file) {
        char command[128] = "globalusage";
        handle_redirect(command, output_file, 0);
    } else {
        printf("IMCSH Version 1.1 created by Leen Al Majzoub and Botond Hernyes\n");
        return 0;
    }
}

int handle_redirect(char *cmd, char *filename, int background) {
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file for redirection");
        return 1;
    }

    if (strcmp(cmd, "globalusage") == 0) {
        char *buffer = "IMCSH Version 1.1 created by Leen Al Majzoub and Botond Hernyes\n";
        write(fd, buffer, strlen(buffer));
        close(fd);
        return 0;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        close(fd);
        return 1;
    } else if (pid == 0) { // Child process
        dup2(fd, STDOUT_FILENO); // Redirect stdout to file
        close(fd); // Close file descriptor

        // Tokenize the command string
        char *args[MAX_ARGS];
        char *token = strtok(cmd, " ");
        int arg_count = 0;
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        // Execute the command
        execvp(args[0], args);
        perror("exec failed"); // Only runs if execvp fails
        exit(1);
    } else { // Parent process
        close(fd); // Parent doesn't need the file descriptor
        if (!background) {
            waitpid(pid, NULL, 0); // Wait for child to finish
        } else {
            printf("Process %d running in background.\n", pid);
        }
    }
    return 0;
}

int handle_quit() {
    // Check for running background processes
    if (bg_process_count > 0) {
        printf("The following processes are running, are you sure you want to quit? [Y/n]\n");
        for (int i = 0; i < bg_process_count; i++) {
            if (background_processes[i] != -1) { // Prints out all of the running processes 🏃‍➡️
                printf("PID: %d\n", background_processes[i]);
            }
        }
        while (1) {
            char response[4];
            fgets(response, sizeof(response), stdin); // Get user input
            if (response[0] == 'Y' || response[0] == 'y') { // If yes
                for (int i = 0; i < bg_process_count; i++) {
                    if (background_processes[i] != -1) {
                        kill(background_processes[i], SIGTERM); // Kills the process we are currently at in the loop
                        printf("Terminated process with PID: %d\n", background_processes[i]);
                        background_processes[i] = -1;
                    }
                }
                return 0;
            } else if (response[0] == 'N' || response[0] == 'n') {  // If no
                printf("Quit aborted.\n");
                return 1;
            } else {    // If the user can't read
                printf("Invalid input. Please enter Y or n.\n");
            }
        }
    }

    printf("Exiting IMCSH. Goodbye!\n");
    exit(0);
}

int main() {
    char command[1024];
    char user[256], host[256];
    getlogin_r(user, sizeof(user)); // Current user
    gethostname(host, sizeof(host)); // Hostname

    while (1) {
        // Check for finished background processes
        check_background_processes();

        printf("%s@%s> ", user, host);
        if (fgets(command, sizeof(command), stdin) == NULL) break; // User input
        command[strcspn(command, "\n")] = '\0';
        if (strlen(command) == 0) continue; // If the command is empty, then skip it


        if (strcmp(command, "quit") == 0) {
            int res = handle_quit();
            if (res == 0) {
                break;
            }
        } else if (strncmp(command, "exec ", 5) == 0) {
            handle_exec(command);
        } else if (strncmp(command, "globalusage", 11) == 0) {
            handle_globalusage(command);
        }
    }
    return 0;
}