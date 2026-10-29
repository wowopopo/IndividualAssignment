#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 256

// Signal handler for handling Ctrl+C (SIGINT)
void handle_sigint(int sig) {
    printf("\n\n==================================================");
    printf("\n[INTERRUPT DETECTED] Interrupt signal (Ctrl + C) received!");
    printf("\nExiting program gracefully...\n");
    printf("==================================================\n");
    exit(0);
}

int main() {
    char input_buf[BUFFER_SIZE];
    int child_count = 0;

    // Register custom signal handler for Ctrl+C
    signal(SIGINT, handle_sigint);

    printf("==================================================\n");
    printf(" Program Started!\n");
    printf(" Type messages continuously to fork new child processes.\n");
    printf(" Press Ctrl + C at any time to interrupt and terminate.\n");
    printf("==================================================\n\n");

    // Infinite loop to keep creating child processes per input
    while (1) {
        int pipefd[2];
        pid_t pid;

        printf("[Parent PID: %d] Enter a message for Child #%d: ", getpid(), child_count + 1);
        fflush(stdout);

        // Read user input from terminal
        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
            break; // Exit loop on EOF (Ctrl+D)
        }

        // Create a new pipe for this specific parent-child exchange
        if (pipe(pipefd) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }

        // Fork into a new child process
        pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // --- CHILD PROCESS ---
            close(pipefd[1]); // Close unused write end

            char child_buf[BUFFER_SIZE];
            ssize_t bytes_read = read(pipefd[0], child_buf, sizeof(child_buf) - 1);

            if (bytes_read > 0) {
                child_buf[bytes_read] = '\0'; // Null-terminate string
                printf("  └─> [Child #%d | PID: %d] Received: %s\n", 
                       child_count + 1, getpid(), child_buf);
            }

            close(pipefd[0]); // Close read end
            exit(EXIT_SUCCESS); // Terminate child after processing
        } else {
            // --- PARENT PROCESS ---
            close(pipefd[0]); // Close unused read end

            // Send message to child via pipe
            write(pipefd[1], input_buf, strlen(input_buf));
            close(pipefd[1]); // Close write end to signal EOF to child

            wait(NULL); // Wait for the child to finish before continuing
            child_count++;
        }
    }

    return 0;
}
