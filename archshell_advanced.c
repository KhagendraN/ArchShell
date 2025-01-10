#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100
#define MAX_HISTORY 100

// Command history
char *history[MAX_HISTORY];
int history_count = 0;

// Signal handling for Ctrl+C
void handle_sigint(int sig) {
    printf("\nUse 'exit' to quit the shell.\n");
}

// Print the shell prompt with the current directory
void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("archshell:%s> ", cwd);
    } else {
        perror("getcwd");
        printf("archshell> ");
    }
    fflush(stdout);
}

// Read user input
void read_input(char *input) {
    if (fgets(input, MAX_INPUT, stdin) == NULL) {
        printf("\n");
        exit(0); 
    }
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0'; 
    }
}

// Parse input into arguments
void parse_input(char *input, char **args) {
    char *token = strtok(input, " ");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // Null-terminate the array
}

// Add command to history
void add_to_history(char *input) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(input);
    } else {
        free(history[0]);
        for (int i = 1; i < MAX_HISTORY; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY - 1] = strdup(input);
    }
}

// Display command history
void show_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Execute built-in commands like cd, exit, or history
int handle_builtin_commands(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "archshell: cd: missing argument\n");
        } else if (chdir(args[1]) != 0) {
            perror("archshell");
        }
        return 1;
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "history") == 0) {
        show_history();
        return 1;
    } else if (strcmp(args[0], "help") == 0) {
        printf("Archshell Help:\n");
        printf("- cd [dir]: Change directory\n");
        printf("- exit: Exit the shell\n");
        printf("- history: Show command history\n");
        printf("- Standard commands with |, >, and < are supported.\n");
        return 1;
    }
    return 0;
}

// Execute commands with piping and redirection
void execute_command(char **args) {
    int i = 0;
    int fd_in, fd_out;

    // Check for redirection or pipe
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return;
            }

            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                dup2(pipefd[1], STDOUT_FILENO); 
                close(pipefd[0]);
                close(pipefd[1]);
                execute_command(args);
                exit(1);
            } else {
                // Parent process
                wait(NULL);
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe
                close(pipefd[0]);
                execute_command(&args[i + 1]);
                return;
            }
        } else if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;
            fd_out = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("archshell");
                return;
            }
            dup2(fd_out, STDOUT_FILENO); // Redirect stdout to file
            close(fd_out);
            break;
        } else if (strcmp(args[i], "<") == 0) {
            args[i] = NULL;
            fd_in = open(args[i + 1], O_RDONLY);
            if (fd_in < 0) {
                perror("archshell");
                return;
            }
            dup2(fd_in, STDIN_FILENO); // Redirect stdin from file
            close(fd_in);
            break;
        }
        i++;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("archshell");
        }
        exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    // Signal handling
    signal(SIGINT, handle_sigint);

    while (1) {
        print_prompt();
        read_input(input);

        if (strlen(input) == 0) {
            continue; // Ignore empty input
        }

        add_to_history(input); // Add input to history

        parse_input(input, args);

        if (args[0] == NULL) {
            continue; // Ignore empty command
        }

        if (!handle_builtin_commands(args)) {
            execute_command(args); // Execute external command
        }
    }

    return 0;
}
