#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(void) {
    char path[1024];
    char ch;

    printf("Enter file path: ");
    fflush(stdout);

    if (fgets(path, sizeof(path), stdin) == NULL) {
        perror("Error reading input");
        return 1;
    }

    path[strcspn(path, "\n")] = '\0';

    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    for (off_t i = size; i > 0; i--) {

        if (lseek(fd, i - 1, SEEK_SET) == -1) {
            perror("Error seeking");
            close(fd);
            return 1;
        }

        ssize_t n;
        do {
            n = read(fd, &ch, 1);
        } while (n == -1 && errno == EINTR);

        if (n != 1) {
            if (n == -1)
                perror("Error reading");
            else
                fprintf(stderr, "Unexpected end of file\n");

            close(fd);
            return 1;
        }

        do {
            n = write(STDOUT_FILENO, &ch, 1);
        } while (n == -1 && errno == EINTR);

        if (n != 1) {
            if (n == -1)
                perror("Error writing");
            else
                fprintf(stderr, "Error: no byte written\n");

            close(fd);
            return 1;
        }
    }

    if (write(STDOUT_FILENO, "\n", 1) != 1) {
        perror("Error writing newline");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }
    return 0;
}
