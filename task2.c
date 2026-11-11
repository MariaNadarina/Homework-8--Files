#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    const char *text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char buffer[11];

    int fd = open("data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    size_t length = strlen(text);
    size_t written = 0;

    while (written < length) {
        ssize_t n = write(fd, text + written, length - written);

        if (n <= 0) {
            if (n == -1)
                perror("Error writing file");
            else
                fprintf(stderr, "Write failed\n");
            close(fd);
            return 1;
        }
        written += n;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    fd = open("data.txt", O_RDWR);

    if (fd == -1) {
        perror("Error reopening file");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    printf("Original size: %lld bytes\n", (long long)size);

    if (ftruncate(fd, 10) == -1) {
        perror("Error truncating file");
        close(fd);
        return 1;
    }

    size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("Error getting new size");
        close(fd);
        return 1;
    }

    printf("New size: %lld bytes\n", (long long)size);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Error rewinding file");
        close(fd);
        return 1;
    }

    size_t total = 0;

    while (total < 10) {
        ssize_t n = read(fd, buffer + total, 10 - total);

        if (n == -1) {
            perror("Error reading file");
            close(fd);
            return 1;
        }

        if (n == 0)
            break;

        total += n;
    }

    buffer[total] = '\0';

    printf("Remaining content: %s\n", buffer);

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}

