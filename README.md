# IMCSH - IMC Shell

## Authors
Leen Al Majzoub and Botond Hernyes

## Overview
IMCSH (IMC Shell) is an alternative Linux shell implemented in C. It interacts with the user via the command line, accepts commands from the user using standard input (stdin), executes the provided commands, and prints the result to the standard output (stdout).

## Features
- **exec <program_to_execute>**: Accepts a program to execute with its parameters and executes it by creating a child process. The program to execute will be a string (eventually with blank spaces in between if command-line parameters are needed). After termination of the child process, the shell will output the PID of the finished process in a new line.
  - Example: `exec ps -uax` will execute the command `ps -uax`.
  - Hint: Use the `strtok` function to tokenize (parse) the input string.

- **Modifier &**: If this modifier is used at the end of an `exec` command, imcsh will execute the command in the background and continue immediately (without waiting for the command to finish). As soon as the process ends, it will output the PID of the finished process in a new line.

- **globalusage**: This internal command will show a string with details on the version of imcsh being executed.
  - Example output: `IMCSH Version 1.1 created by Leen Al Majzoub and Botond Hernyes`.

- **Modifier >**: If this modifier is used at the end of a command, imcsh will redirect the output as text to the file given after the symbol. If the file does not exist, the shell will automatically create it. If the file exists, the shell will try to append the new data to the end of the file. In this case, the only visible output in the console will be the PID of the process after termination.
  - Example 1: `exec ls -l > directory_output.txt` → Executes the command `ls -l` and writes the output to the file `directory_output.txt`.
  - Example 2: `globalusage > usage.txt` → Executes the command `globalusage` and writes the output to the file `usage.txt`.
  - Hint: Use the `dup2` system call (unistd.h) to redirect standard output (stdout) to a file descriptor.

- **quit**: Quits the shell. If there are any running processes, the following question will be prompted to the user: `The following processes are running, are you sure you want to quit? [Y/n]`. A list of all currently running processes will follow. If the user enters `Y`, the shell will quit and all running processes will be terminated.

## Usage
Run the shell using the following command:
```sh
make run