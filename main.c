#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_PROCESSES 128
pid_t background_processes[MAX_PROCESSES]; // Array to track background PIDs
int bg_process_count = 0;

int main() {
    char command[1024];
    char user[256], host[256];
    getlogin_r(user, sizeof(user)); // Current user
    gethostname(host, sizeof(host)); // Hostname

    while (1) {
        printf("%s@%s> ", user, host);
        if (fgets(command, sizeof(command), stdin) == NULL) break; // User input
        command[strcspn(command, "\n")] = '\0';
        if (strlen(command) == 0) continue; // If the command is empty, then skip it


        if (strcmp(command, "quit") == 0) {
            break;
        } else if (strncmp(command, "exec ", 5) == 0) {
            handle_exec(command);
        } else if (strcmp(command, "globalusage") == 0) {
            // TODO: globalusage
        }
    }
    return 0;
}

void handle_exec(char *cmd) {
    char *args[128];
    int background = 0;

    char *token = strtok(cmd, " ");
    int count = 0;
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            background = 1; // Background process
            break;
        }
        args[count++] = token;
        token = strtok(NULL, " ");  // Parsing as the hint said
    }
    args[count] = NULL; // Null-terminate the arguments 😵

    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (execvp(args[0], args) == -1) {
            perror("exec failed");
        }
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // Parent process
        if (!background) {
            waitpid(pid, NULL, 0); // Waiting for child to finish...
        } else {
            if (bg_process_count < MAX_PROCESSES) {
                background_processes[bg_process_count++] = pid; // Add PID to the array
                printf("Background process started with PID: %d\n", pid);
            } else {
                printf("Maximum background process limit reached.\n");
            }
        }
    } else {
        perror("fork failed");
    }
}

void handle_globalusage() {
    printf("IMCSH Version 1.1 created by Leen Al Majzoub and Botond Hernyes\n");
}

void handle_redirect(char *cmd) {
    char *file = strchr(cmd, '>');
    if (file != NULL) {
        *file = '\0'; // Splitting the command and the ile
        file++;
        while (*file == ' ') file++; // Leaves out the spaces
        int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1) {
            perror("Failed to open file");
            return;
        }
        int stdout_copy = dup(1);   // Backup stdout 🤓
        dup2(fd, 1);                // Redirect stdout to file
        handle_exec(cmd);
        dup2(stdout_copy, 1);       // Restore stdout 🔁
        close(fd);
    } else {
        handle_exec(cmd);
    }
}

void handle_quit() {
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
            } else if (response[0] == 'N' || response[0] == 'n') {  // If no
                printf("Quit aborted.\n");
                return;
            } else {    // If the user can't read
                printf("Invalid input. Please enter Y or n.\n");
            }
        }
    }

    printf("Exiting IMCSH. Goodbye!\n");
    exit(0);
}