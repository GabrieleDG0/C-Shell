# Custom Shell

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

    ![image](https://github.com/user-attachments/assets/c9dc0a7f-6ad2-4d59-aa86-7f5b07405b7f)

    The read_input() function reads a line from the user and supports character-by-character input with real-time display. It processes special keys such as the backspace key to process the command.

2. Parsing commands
    The shell supports multiple commands separated by pipes (|), redirection operators (> and <) and spaces to separate arguments.
    The input is broken down into commands and their arguments, and redirection files are identified.

    Command structure:
    command struct stores the command name, arguments and redirection files for the input/output.
    commands struct contains an array of commands for parsing multiple commands in a pipeline.

   Example:
   
   ![image](https://github.com/user-attachments/assets/72da56a2-4e82-473d-a7df-b520eb35e3b8)

    The parser processes the following commands:
    1. Extracting command names and arguments.
    2. Identification of redirection operators (<, >).
    3. Handling multiple commands connected by pipes.

4. Execution of commands
    The exec_command() function uses fork() to create a new process and execvp() to execute the command in that process. Redirections are handled by changing the file descriptors before the command is executed:
    - Input redirection: dup2() is used to redirect standard input (STDIN_FILENO) to a file.
    - Output redirection: Similarly, dup2() redirects the standard output (STDOUT_FILENO) to a file.
    
    Example for handling the redirection:
   
    ![image](https://github.com/user-attachments/assets/c41d2f30-e544-493b-aac2-1364ec3309b1)

3. Piping
    For commands that are connected to pipes, the shell creates a series of pipes with pipe(). It then redirects the output of a command to the input of the next command with dup2() before executing each command in a separate process with fork().

    Example of setting up pipes:
   
    ![image](https://github.com/user-attachments/assets/127b88c8-ffde-4286-8570-d551dad590ab)

    Each subordinate process inherits its respective input/output redirects from the pipe.

5. Built-in commands
    The shell contains several built-in commands that are processed before attempting to execute external commands:
    - exit: Exits the shell and clears the history.
      
    ![image](https://github.com/user-attachments/assets/a9b31215-3097-436d-874b-969978805714)

    - cd: Changes the current directory. If no argument is specified, an error message is displayed.
      
    ![image](https://github.com/user-attachments/assets/20aa0264-4a2d-41b6-8031-ff638523f2ea)

    - history: Displays the command history. The user can also re-execute a command from the history by specifying its index.
      
    ![image](https://github.com/user-attachments/assets/d5079374-d452-468a-8dda-f4c83405dc53)

6. Managing the history
    The shell manages a history of up to 100 commands. Each new command is saved in the history array with add_to_history(). If the history exceeds the limit, the oldest commands are removed.
    The commands in the history can be accessed with the history command:
   
    ![image](https://github.com/user-attachments/assets/61bf001f-dbc2-4e89-a786-c9c6def43a4e)

7. Signal processing
    The shell is designed to handle SIGINT (Ctrl+C), which terminates most processes by default, appropriately. The handle_signal() function ensures that the shell is not terminated when this signal is received, providing a more user-friendly experience:
   
    ![image](https://github.com/user-attachments/assets/67ae0821-76e7-486a-a1be-394f6030c916)

8. Main loop
    The main loop of the shell repeatedly reads user input, parses the commands, processes integrated commands and executes external commands. The program continues to run until the exit command is invoked.

## Example Usage

![image](https://github.com/user-attachments/assets/4e520a5d-5042-446d-82ae-b247e52dc30f)

### Contributions
Feel free to fork this repository, open issues, or submit pull requests to improve the project. Contributions can range from adding new features to optimizing the existing code.
