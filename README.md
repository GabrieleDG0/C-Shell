# Custom Shell

## Introduction
A fully custom shell written in C. This project is a simple implementation of a Unix-like shell capable of executing commands, managing command history, handling piping and redirection, and supporting built-in commands like cd, exit, and history. It also includes basic signal handling to manage interruptions like SIGINT (Ctrl+C).

### Features
- Command execution: Executes user input as system commands.
- Redirection: Handles input (<) and output (>) redirection.
- Piping: Supports piping between multiple commands (|).
- Command history: Stores and displays the last 100 commands, allowing re-execution.
- Built-in commands: Includes support for exit, cd, and history.
- Signal handling: Proper handling of SIGINT to prevent termination when Ctrl+C is pressed.

## Setup and Initialization

### Library Inclusions:
The program uses standard C libraries like stdio.h, stdlib.h, unistd.h, fcntl.h, string.h, sys/wait.h, and signal.h for managing input/output, memory, process control, and signals.

### History Management:
A fixed-size array (history[MAX_HISTORY]) stores the last 100 commands typed by the user.
When the history exceeds this limit, older commands are removed to make room for new ones.

### Signal Handling:
The shell traps SIGINT (Ctrl+C) to prevent immediate termination of the program. This makes the shell more resilient to interruptions during execution.

## Functionality
1. Input Reading and Prompting
The shell displays a prompt showing the current working directory:

printf("[%s] > ", getcwd(NULL, 0));
The read_input() function reads a line from the user, supporting character-by-character input with real-time display. It handles special keys like the backspace for editing the command.

2. Command Parsing
The shell supports multiple commands separated by pipes (|), redirection operators (> and <), and spaces to separate arguments. The input is parsed into commands and their arguments, and redirection files are identified.

Command structure:
command struct stores the command name, arguments, and input/output redirection files.
commands struct holds an array of commands for parsing multiple commands in a pipeline.
Example:

int parse_command(char *input, command *cmd)
The parser handles the following:

Extracting command names and arguments.
Identifying redirection operators (<, >).
Handling multiple commands connected by pipes.
3. Command Execution
The exec_command() function uses fork() to create a new process and execvp() to execute the command in that process. Redirections are handled by modifying the file descriptors before the command is executed:

Input Redirection: dup2() is used to redirect the standard input (STDIN_FILENO) to a file.
Output Redirection: Similarly, dup2() redirects the standard output (STDOUT_FILENO) to a file.
Example of redirection handling:

if (cmd->input_file != NULL) {
    int fd_in = open(cmd->input_file, O_RDONLY);
    dup2(fd_in, STDIN_FILENO);
    close(fd_in);
}
4. Piping
For commands connected with pipes, the shell creates a series of pipes using pipe(). It then redirects the output of one command to the input of the next using dup2() before executing each command in a separate process via fork().

Example of setting up piping:

if (pipe(pipefds + i * 2) == -1) {
    perror("Pipe creation failed");
    exit(EXIT_FAILURE);
}
Each child process will inherit its respective input/output redirections from the pipe.

5. Built-in Commands
The shell includes several built-in commands, which are processed before attempting to execute external commands:

exit: Terminates the shell and clears the history.

if (strcmp(cmd->name, "exit") == 0) {
    clear_history();
    exit(0);
}
cd: Changes the current directory. If no argument is provided, it displays an error message.

if (strcmp(cmd->name, "cd") == 0) {
    chdir(cmd->argv[1]);
}
history: Displays the command history. Users can also re-execute a command from history by specifying its index.

if (strcmp(cmd->name, "history") == 0) {
    print_history();
}
6. History Management
The shell maintains a history of up to 100 commands. Each new command is stored in the history array using add_to_history(). If the history exceeds the limit, the oldest commands are removed.

Commands in history can be accessed using the history command:

void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}
7. Signal Handling
The shell is designed to gracefully handle SIGINT (Ctrl+C), which by default terminates most processes. The handle_signal() function ensures that the shell does not terminate when this signal is received, allowing for a more user-friendly experience:

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\n");
    }
}
8. Main Loop
The main loop of the shell repeatedly reads user input, parses the commands, handles built-in commands, and executes external commands. The program continues running until the exit command is invoked.

## Example Usage
![image](https://github.com/user-attachments/assets/11add84c-2d55-4fd4-be76-ae6d5bf81947)


### Contributions
Feel free to fork this repository, open issues, or submit pull requests to improve the project. Contributions can range from adding new features to optimizing the existing code.
