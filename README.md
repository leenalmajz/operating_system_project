# IMCSH - IMC Shell

## Authors
Leen Al Majzoub and Botond Hernyes

## Overview
IMCSH (IMC Shell) is an alternative Linux shell implemented in C. It interacts with the user via the command line, accepts commands from the user using standard input (stdin), executes the provided commands, and prints the result to the standard output (stdout).

## Features
- **exec <program_to_execute>**: Executes a program with its parameters by creating a child process.
- **&**: Executes the command in the background and continues immediately.
- **globalusage**: Displays details on the version of IMCSH being executed.
- **>**: Redirects the output of a command to a file.
- **quit**: Quits the shell, optionally terminating any running background processes.

## Usage
Run the shell using the following command:
```sh
make run