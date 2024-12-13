#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

#define MAX_HISTORY 100
#define MAX_COMMAND_LENGTH 1024
#define MAX_TOKENS 100

typedef struct {
    char *name;              // Command name
    char *argv[MAX_TOKENS];  // Array of command arguments
    char *input_file;        // Input redirection file
    char *output_file;       // Output redirection file
} command;

typedef struct {
    int num_commands;         // Number of commands in the pipeline
    command cmds[MAX_TOKENS]; // Array of commands
} commands;

char *history[MAX_HISTORY];  // History of commands
int history_count = 0;       // Number of commands in history

// Add a command to the history
void add_to_history(const char *input) {
    if (history_count >= MAX_HISTORY) {
        free(history[0]);
        memmove(history, history + 1, (MAX_HISTORY - 1) * sizeof(char *));
        history_count--;
    }
    history[history_count++] = strdup(input);
}

// Free all allocated memory in history
void clear_history() {
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    history_count = 0;
}

// Print the command history
void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Read input from the user
void read_input(char *input) {
    printf("[%s] > ", getcwd(NULL, 0)); // Prompt
    fflush(stdout);

    int pos = 0; // Position in the input buffer
    input[0] = '\0';

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == -1) {
            perror("read() error");
            exit(EXIT_FAILURE);
        }

        if (c == '\n') { // Enter key
            input[pos] = '\0';
            printf("\n");
            break;
        }

        if (isprint(c)) { // Printable characters
            input[pos++] = c;
            input[pos] = '\0';
            printf("%c", c);
        } else if (c == 127 && pos > 0) { // Backspace
            pos--;
            input[pos] = '\0';
            printf("\r\033[K[%s] > %s", getcwd(NULL, 0), input);
        }
    }
}

// Parse a single command into its name, arguments, and redirections
int parse_command(char *input, command *cmd) {
    char *token = strtok(input, " ");
    int token_index = 0;
    cmd->name = NULL;
    cmd->input_file = NULL;
    cmd->output_file = NULL;

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Error: Missing input file for redirection\n");
                return -1;
            }
            cmd->input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Error: Missing output file for redirection\n");
                return -1;
            }
            cmd->output_file = token;
        } else {
            if (cmd->name == NULL) {
                cmd->name = token;
            }
            cmd->argv[token_index++] = token;
        }
        token = strtok(NULL, " ");
    }
    cmd->argv[token_index] = NULL;
    return 0;
}

// Parse the input string into a sequence of commands separated by pipes
int parse_commands_with_pipes(char *input, commands *cmds) {
    char *token = strtok(input, "|");
    int command_index = 0;
    while (token != NULL) {
        if (parse_command(token, &cmds->cmds[command_index]) == -1) {
            return -1;
        }
        command_index++;
        token = strtok(NULL, "|");
    }
    cmds->num_commands = command_index;
    return command_index;
}

// Execute a single command with redirections using execvp
void exec_command(command *cmd) {
    if (cmd->input_file != NULL) {
        int fd_in = open(cmd->input_file, O_RDONLY);
        if (fd_in == -1) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if (cmd->output_file != NULL) {
        int fd_out = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out == -1) {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
    if (execvp(cmd->name, cmd->argv) == -1) {
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    }
}

// Execute a sequence of commands with pipes and redirections
void exec_commands(commands *cmds) {
    int pipefds[2 * (cmds->num_commands - 1)];
    pid_t pids[cmds->num_commands];

    for (int i = 0; i < cmds->num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < cmds->num_commands; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < cmds->num_commands - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }
            for (int j = 0; j < 2 * (cmds->num_commands - 1); j++) {
                close(pipefds[j]);
            }
            exec_command(&cmds->cmds[i]);
        }
    }

    for (int i = 0; i < 2 * (cmds->num_commands - 1); i++) {
        close(pipefds[i]);
    }

    for (int i = 0; i < cmds->num_commands; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

// Handle built-in commands such as exit, cd, and history
int handle_built_in(command *cmd) {
    if (strcmp(cmd->name, "exit") == 0) {
        clear_history();
        exit(0);
    } else if (strcmp(cmd->name, "cd") == 0) {
        if (cmd->argv[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(cmd->argv[1]) == -1) {
                perror("cd failed");
            }
        }
        return 1;
    } else if (strcmp(cmd->name, "history") == 0) {
        if (cmd->argv[1] != NULL) {
            int index = atoi(cmd->argv[1]) - 1;
            if (index >= 0 && index < history_count) {
                printf("%s\n", history[index]);
                command cmd_to_run;
                parse_command(history[index], &cmd_to_run);
                pid_t pid = fork();
                if (pid == -1) {
                    perror("Fork failed");
                    return 1;
                } else if (pid == 0) {
                    exec_command(&cmd_to_run);
                } else {
                    waitpid(pid, NULL, 0);
                }
            } else {
                fprintf(stderr, "Invalid history index\n");
            }
        } else {
            print_history();
        }
        return 1;
    }
    return 0;
}

// Handle signals such as SIGINT
void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\n");
    }
}

int main() {
    char input[MAX_COMMAND_LENGTH];
    commands cmds;

    signal(SIGINT, handle_signal);

    while (1) {
        read_input(input);
        if (strlen(input) == 0) {
            continue;
        }

        add_to_history(input);

        if (parse_commands_with_pipes(input, &cmds) == -1) {
            continue;
        }

        if (handle_built_in(&cmds.cmds[0])) {
            continue;
        }

        exec_commands(&cmds);
    }

    clear_history();
    return 0;
}

