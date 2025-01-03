# Custom Linux Shell

## Introduction
This project is a simple implementation of a Unix-like shell capable of executing commands, managing command history, handling piping and redirection, and supporting built-in commands such as cd, exit and history. It also includes basic signal processing to manage interrupts such as SIGINT (Ctrl+C).

### Features
- Command execution: Executes user input as system commands.
- Redirection: Allows input to be redirected (<) and output (>).
- Piping: Supports piping between multiple commands (|).
- Command history: Saves and displays the last 100 commands and enables re-execution.
- Built-in commands: Includes support for exit, cd and history.
- Signal handling: Proper handling of SIGINT to prevent abort when Ctrl+C is pressed.

## Setup and initialization

### Integration of libraries:
The program uses standard C libraries such as stdio.h, stdlib.h, unistd.h, fcntl.h, string.h, sys/wait.h and signal.h to manage input/output, memory, process control and signals.

### History management:
An array of fixed size (history[MAX_HISTORY]) stores the last 100 commands entered by the user.
If the history exceeds this limit, older commands are removed to make room for new commands.

### Signal processing:
The shell intercepts SIGINT (Ctrl+C) to prevent the program from terminating immediately. This makes the shell more resistant to interruptions during execution.

## Functionality
1. Reading input and prompting
    The shell displays a prompt showing the current working directory:

```
    printf("[%s] > ", getcwd(NULL, 0));
```

The read_input() function reads a line from the user and supports character-by-character input with real-time display. It processes special keys such as the backspace key to process the command.

3. Parsing commands
    The shell supports multiple commands separated by pipes (|), redirection operators (> and <) and spaces to separate arguments.
    The input is broken down into commands and their arguments, and redirection files are identified.

    Command structure:
    command struct stores the command name, arguments and redirection files for the input/output.
    commands struct contains an array of commands for parsing multiple commands in a pipeline.

   Example:
   
```
   int parse_command(char *input, command *cmd)
```

The parser processes the following commands:
    1. Extracting command names and arguments.
    2. Identification of redirection operators (<, >).
    3. Handling multiple commands connected by pipes.

4. Execution of commands
    The exec_command() function uses fork() to create a new process and execvp() to execute the command in that process. Redirections are handled by changing the file descriptors before the command is executed:
    - Input redirection: dup2() is used to redirect standard input (STDIN_FILENO) to a file.
    - Output redirection: Similarly, dup2() redirects the standard output (STDOUT_FILENO) to a file.
    
    Example for handling the redirection:
   
```
    if (cmd->input_file != NULL) {
        int fd_in = open(cmd->input_file, O_RDONLY);
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
```

3. Piping
    For commands that are connected to pipes, the shell creates a series of pipes with pipe(). It then redirects the output of a command to the input of the next command with dup2() before executing each command in a separate process with fork().

    Example of setting up pipes:
   
```
    if (pipe(pipefds + i * 2) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }
```

Each subordinate process inherits its respective input/output redirects from the pipe.

5. Built-in commands
    The shell contains several built-in commands that are processed before attempting to execute external commands:
    - exit: Exits the shell and clears the history.

```
    if (strcmp(cmd->name, "exit") == 0) {
        clear_history();
        exit(0);
    }
```

- cd: Changes the current directory. If no argument is specified, an error message is displayed.
      
```
    if (strcmp(cmd->name, "cd") == 0) {
        chdir(cmd->argv[1]);
    }
```

- history: Displays the command history. The user can also re-execute a command from the history by specifying its index.
      
```
    if (strcmp(cmd->name, "history") == 0) {
        print_history();
    }
```

6. Managing the history
    The shell manages a history of up to 100 commands. Each new command is saved in the history array with add_to_history(). If the history exceeds the limit, the oldest commands are removed.
    The commands in the history can be accessed with the history command:
   
```
   void print_history() {
       for (int i = 0; i < history_count; i++) {
           printf("%d: %s\n", i + 1, history[i]);
       }
   }
```

7. Signal processing
    The shell is designed to handle SIGINT (Ctrl+C), which terminates most processes by default, appropriately. The handle_signal() function ensures that the shell is not terminated when this signal is received, providing a more user-friendly experience:

```
    void handle_signal(int sig) {
        if (sig == SIGINT) {
            printf("\n");
        }
    }
```

8. Main loop
    The main loop of the shell repeatedly reads user input, parses the commands, processes integrated commands and executes external commands. The program continues to run until the exit command is invoked.

## Example Usage

```
    $ ./shell
    [/home/user] > ls
    [/home/user] > cd /var
    [/var] > ls -l > output.txt
    [/var] > cat output.txt
    [/var] > history
    1: ls
    2: cd /var
    3: ls -l > output.txt
    4: cat ouput.txt
    5: history
```

### Contributions
Feel free to fork this repository, open issues, or submit pull requests to improve the project. Contributions can range from adding new features to optimizing the existing code.
