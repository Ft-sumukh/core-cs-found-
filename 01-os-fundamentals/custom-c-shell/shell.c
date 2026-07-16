#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 100

int main() {
    char input[MAX_COMMAND_LENGTH];
    char *args[MAX_ARGS];

    while (1) {
        printf("sumukh-shell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) {
            continue;
        }

        if (strcmp(input, "exit") == 0) {
            break;
        }

        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; 

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
        } else if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("Command execution failed");
            }
            exit(EXIT_FAILURE); 
        } else {
            wait(NULL);
        }
    }

    return 0;
}