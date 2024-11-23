#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

int main() {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = 0;
    size_t total_bytes = 0;
    
    // Read from stdin until EOF
    while ((bytes_read = read(STDIN_FILENO, buffer + total_bytes, 
           BUFFER_SIZE - total_bytes)) > 0) {
        total_bytes += bytes_read;
        
        // Reallocate buffer if needed
        if (total_bytes >= BUFFER_SIZE - 1) {
            // For simplicity, we'll just stop if buffer is full
            break;
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from stdin");
        return 1;
    }

    // Write bytes in reverse order to stdout
    for (ssize_t i = total_bytes - 1; i >= 0; i--) {
        if (write(STDOUT_FILENO, &buffer[i], 1) != 1) {
            perror("Error writing to stdout");
            return 1;
        }
    }

    return 0;
}