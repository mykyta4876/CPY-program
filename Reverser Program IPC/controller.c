#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LINES 100
#define MAX_LINE_LENGTH 1024
#define BUFFER_SIZE 4096

typedef struct {
    int read_fd;
    int write_fd;
} PipePair;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file.usp> <output_file>\n", argv[0]);
        return 1;
    }

    // Open input file
    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        perror("Error opening input file");
        return 1;
    }

    // Open output file
    int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("Error opening output file");
        close(input_fd);
        return 1;
    }

    // Read input file line by line
    char buffer[BUFFER_SIZE];
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    PipePair pipes[MAX_LINES];
    int line_count = 0;
    ssize_t bytes_read;
    int current_pos = 0;

    while ((bytes_read = read(input_fd, buffer + current_pos, 1)) > 0) {
        if (buffer[current_pos] == '\n' || current_pos == MAX_LINE_LENGTH - 1) {
            buffer[current_pos] = '\0';
            strncpy(lines[line_count], buffer, MAX_LINE_LENGTH);
            line_count++;
            current_pos = 0;
        } else {
            current_pos++;
        }
    }

    // Handle last line if it doesn't end with newline
    if (current_pos > 0) {
        buffer[current_pos] = '\0';
        strncpy(lines[line_count], buffer, MAX_LINE_LENGTH);
        line_count++;
    }

    // Create pipes and child processes
    for (int i = 0; i < line_count; i++) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("Pipe creation failed");
            return 1;
        }
        pipes[i].read_fd = pipefd[0];
        pipes[i].write_fd = pipefd[1];

        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            return 1;
        }

        if (pid == 0) { // Child process
            close(pipes[i].write_fd);
            dup2(pipes[i].read_fd, STDIN_FILENO);
            close(pipes[i].read_fd);

            // Create pipe for output
            int output_pipe[2];
            if (pipe(output_pipe) == -1) {
                perror("Output pipe creation failed");
                return 1;
            }

            pid_t child_pid = fork();
            if (child_pid == -1) {
                perror("Second fork failed");
                return 1;
            }

            if (child_pid == 0) {
                close(output_pipe[0]);
                dup2(output_pipe[1], STDOUT_FILENO);
                close(output_pipe[1]);

                char *args[] = {"./reverser", NULL};
                execve("./reverser", args, NULL);
                perror("Execve failed");
                exit(1);
            }

            close(output_pipe[1]);
            
            // Read from child and write to parent's stdout
            char buf[BUFFER_SIZE];
            ssize_t n;
            while ((n = read(output_pipe[0], buf, BUFFER_SIZE)) > 0) {
                write(STDOUT_FILENO, buf, n);
            }
            
            close(output_pipe[0]);
            exit(0);
        }

        // Parent process
        close(pipes[i].read_fd);
        write(pipes[i].write_fd, lines[i], strlen(lines[i]));
        close(pipes[i].write_fd);
    }

    // Wait for all children to complete
    for (int i = 0; i < line_count; i++) {
        wait(NULL);
    }

    // Read results and write to output file
    char result_buffer[BUFFER_SIZE];
    for (int i = line_count - 1; i >= 0; i--) {
        ssize_t n;
        while ((n = read(pipes[i].read_fd, result_buffer, BUFFER_SIZE)) > 0) {
            write(output_fd, result_buffer, n);
        }
        write(output_fd, "\n", 1);
    }

    // Cleanup
    close(input_fd);
    close(output_fd);

    return 0;
}