#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_TOKENS 64

int main() {
    char input[MAX_INPUT];
    char *tokens[MAX_TOKENS];
    char *token;
    char *argv[MAX_TOKENS];
    int token_count = 0;
    pid_t pid;

    while (1) {
        // Print the shell prompt
        printf("myshell$ ");
        fflush(stdout);

        // Read input from the user
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove the newline character from the input
        input[strcspn(input, "\n")] = '\0';

        // If input is empty, continue to the next iteration
        if (strlen(input) == 0)
            continue;

        // Tokenize the input
        token = strtok(input, " ");
        while (token != NULL && token_count < MAX_TOKENS - 1) {
            argv[token_count++] = token;
            token = strtok(NULL, " ");
        }
        argv[token_count] = NULL;

        // Handle built-in commands here if needed

        // Fork a child process to execute the command
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            // Child process
            if (execvp(argv[0], argv) < 0) {
                perror("execvp failed");
                exit(1);
            }
        } else {
            // Parent process
            wait(NULL);
        }

        // Reset token count for the next command
        token_count = 0;
    }

    return 0;
}
