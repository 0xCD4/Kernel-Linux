#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>

#define HISTORY_SIZE 100
#define MAX_TOKENS 64

struct termios original_termios;
char *history[HISTORY_SIZE];
int history_index = -1;
int current_history = -1;

void set_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void reset_terminal_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// Signal handler for SIGINT
void sigint_handler(int sig) {
    printf("\nmyshell$ ");
    fflush(stdout);
}

char *my_readline() {
    set_raw_mode();
    char *buffer = malloc(1);
    size_t capacity = 1;
    size_t length = 0;
    int ch;
    int state = 0; // 0: normal, 1: esc, 2: esc_bracket

    while (1) {
        ch = getchar();
        if (ch == EOF) {
            reset_terminal_mode();
            free(buffer);
            return NULL;
        }
        if (state == 0) {
            if (ch == 27) { // Escape character
                state = 1;
            } else if (ch == '\n') {
                buffer[length] = '\0';
                // Add to history
                if (length > 0) {
                    if (current_history == -1 || strcmp(buffer, history[current_history]) != 0) {
                        current_history = (current_history + 1) % HISTORY_SIZE;
                        history[current_history] = strdup(buffer);
                    }
                }
                reset_terminal_mode();
                return buffer;
            } else if (ch == 127) { // Backspace
                if (length > 0) {
                    length--;
                    printf("\b \b");
                    buffer[length] = '\0';
                }
            } else {
                if (length >= capacity) {
                    capacity *= 2;
                    buffer = realloc(buffer, capacity);
                }
                buffer[length++] = ch;
                printf("%c", ch);
            }
        } else if (state == 1) {
            if (ch == '[') {
                state = 2;
            } else {
                state = 0;
            }
        } else if (state == 2) {
            if (ch == 'A') { // Up arrow
                if (current_history >= 0) {
                    current_history--;
                    if (current_history < 0) {
                        current_history = HISTORY_SIZE - 1;
                    }
                    if (history[current_history]) {
                        // Clear current line
                        printf("\x1b[2K\x1b[0E");
                        // Print the previous command
                        printf("%s", history[current_history]);
                        // Set buffer to the previous command
                        free(buffer);
                        buffer = strdup(history[current_history]);
                        length = strlen(buffer);
                    }
                }
            } else if (ch == 'B') { // Down arrow
                if (current_history >= 0) {
                    current_history++;
                    if (current_history >= HISTORY_SIZE) {
                        current_history = 0;
                    }
                    if (history[current_history]) {
                        // Clear current line
                        printf("\x1b[2K\x1b[0E");
                        // Print the next command
                        printf("%s", history[current_history]);
                        // Set buffer to the next command
                        free(buffer);
                        buffer = strdup(history[current_history]);
                        length = strlen(buffer);
                    }
                }
            }
            state = 0;
        }
    }
}

int main() {
    signal(SIGINT, sigint_handler);

    char *input;
    char *argv[MAX_TOKENS];
    int token_count = 0;
    pid_t pid;

    while (1) {
        // Print the shell prompt
        printf("myshell$ ");
        fflush(stdout);

        // Read input from the user
        input = my_readline();
        if (input == NULL) {
            printf("\n");
            break;
        }

        // Tokenize the input
        char *token = strtok(input, " ");
        while (token != NULL && token_count < MAX_TOKENS - 1) {
            argv[token_count++] = token;
            token = strtok(NULL, " ");
        }
        argv[token_count] = NULL;

        // Handle built-in commands
        if (token_count > 0) {
            if (strcmp(argv[0], "cd") == 0) {
                if (argv[1] == NULL) {
                    chdir(getenv("HOME"));
                } else {
                    chdir(argv[1]);
                }
            } else if (strcmp(argv[0], "exit") == 0) {
                free(input);
                exit(0);
            } else if (strcmp(argv[0], "history") == 0) {
                for (int i = 0; i < HISTORY_SIZE; i++) {
                    int index = (current_history - i + HISTORY_SIZE) % HISTORY_SIZE;
                    if (history[index]) {
                        printf("%d %s\n", i+1, history[index]);
                    }
                }
            } else {
                // Check for background execution
                int background = 0;
                int argc = token_count;
                if (strcmp(argv[argc-1], "&") == 0) {
                    background = 1;
                    argv[argc-1] = NULL;
                }
                // Reset terminal mode before executing
                reset_terminal_mode();
                // Fork and execute
                pid = fork();
                if (pid < 0) {
                    perror("fork failed");
                } else if (pid == 0) {
                    // Child process
                    execvp(argv[0], argv);
                    perror("execvp failed");
                    exit(1);
                } else {
                    // Parent process
                    if (!background) {
                        wait(NULL);
                    }
                }
            }
        }
        free(input);
        token_count = 0;
    }

    return 0;
}
