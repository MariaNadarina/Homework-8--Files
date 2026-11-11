#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    char source[1024];
    char destination[1024];
    char buffer[4096];

    printf("Enter source file path: ");
    fflush(stdout);

    if (fgets(source, sizeof(source), stdin) == NULL) {
        fprintf(stderr, "Error reading source path\n");
        return 1;
    }

    printf("Enter destination file path: ");
    fflush(stdout);

    if (fgets(destination, sizeof(destination), stdin) == NULL) {
        fprintf(stderr, "Error reading destination path\n");
        return 1;
    }

    source[strcspn(source, "\n")] = '\0';
    destination[strcspn(destination, "\n")] = '\0';

    int src = open(source, O_RDONLY);

    if (src == -1) {
        perror("Error opening source file");
        return 1;
    }

    int dest = open(destination,
                    O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (dest == -1) {
        perror("Error opening destination file");
        if (close(src) == -1)
            perror("Error closing source file");
        return 1;
    }

    unsigned long long total = 0;
    int failed = 0;

    while (1) {
        ssize_t bytes_read = read(src, buffer, sizeof(buffer));

        if (bytes_read == -1) {
            if (errno == EINTR)
                continue;

            perror("Error reading file");
            failed = 1;
            break;
        }

        if (bytes_read == 0)
            break;  // End of file

        ssize_t offset = 0;

        while (offset < bytes_read) {
            ssize_t bytes_written = write(
                dest, buffer + offset, bytes_read - offset
            );

            if (bytes_written == -1) {
                if (errno == EINTR)
                    continue;

                perror("Error writing file");
                failed = 1;
                break;
            }

            if (bytes_written == 0) {
                fprintf(stderr, "Error: write made no progress\n");
                failed = 1;
                break;
            }

            offset += bytes_written;
            total += bytes_written;
        }

        if (failed)
            break;
    }

    if (close(src) == -1) {
        perror("Error closing source file");
        failed = 1;
    }

    if (close(dest) == -1) {
        perror("Error closing destination file");
        failed = 1;
    }

    if (failed)
        return 1;

    printf("Successfully copied %llu bytes.\n", total);

    return 0;
}

