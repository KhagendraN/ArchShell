#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <python3.13/Python.h> 

#define ARCHSHELL_MAX_INPUT 1024
#define MAX_ARGS 100

// Function to handle signals
void sigint_handler(int sig) {
    // Ignore the signal (do nothing)
    printf("\n(Enter 'exit' to exit the shell)\n");
    fflush(stdout);
}

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

void read_input(char *input) {
    if (fgets(input, ARCHSHELL_MAX_INPUT, stdin) == NULL) {
        printf("\n");
        exit(0);
    }
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }
}

void parse_input(char *input, char **args) {
    char *token = strtok(input, " ");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

// Function to run Python scripts
void execute_python_script(const char *script) {
    Py_Initialize(); 

    FILE *fp = fopen(script, "r");
    if (fp == NULL) {
        perror("archshell (python)");
    } else {
        PyRun_SimpleFile(fp, script); 
        fclose(fp);
    }

    Py_Finalize(); 
}

// Detect if the command is Python
int is_python_script(char *input) {
    return strstr(input, ".py") != NULL;
}

// Execute commands
void execute_command(char **args) {
    if (args[0] == NULL) {
        return; 
    }

    // Handle the 'exit' command
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0); 
    }

    if (is_python_script(args[0])) {
        execute_python_script(args[0]); 
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("archshell");
        }
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

int main() {
    // Set the signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, sigint_handler);

    char input[ARCHSHELL_MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        print_prompt();
        read_input(input);

        if (strlen(input) == 0) {
            continue;
        }

        parse_input(input, args);

        execute_command(args);
    }

    return 0;
}
