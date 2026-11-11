#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(void) {

    int fd = open("sparse.bin",
                  O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    const char start[] = "START";
    size_t written = 0;

    while (written < sizeof(start) - 1) {
        ssize_t n = write(fd, start + written,
                          sizeof(start) - 1 - written);

        if (n == -1 && errno == EINTR)
            continue;

        if (n <= 0) {
            perror("Error writing START");
            close(fd);
            return 1;
        }

        written += n;
    }

    if (lseek(fd, 1024 * 1024, SEEK_CUR) == -1) {
        perror("Error seeking");
        close(fd);
        return 1;
    }

    const char end[] = "END";
    written = 0;

    while (written < sizeof(end) - 1) {
        ssize_t n = write(fd, end + written,
                          sizeof(end) - 1 - written);

        if (n == -1 && errno == EINTR)
            continue;

        if (n <= 0) {
            perror("Error writing END");
            close(fd);
            return 1;
        }

        written += n;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    fd = open("sparse.bin", O_RDONLY);

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

    printf("Apparent file size: %lld bytes\n",
           (long long)size);

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}
