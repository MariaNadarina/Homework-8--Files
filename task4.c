#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(void) {
    char line[1024];
    char entry[sizeof(line) + 64];
    size_t len = 0;

    int fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    printf("Enter a log message: ");
    fflush(stdout);

    while (1) {
        char ch;
        ssize_t n = read(0, &ch, 1);

        if (n == -1) {
            if (errno == EINTR)
                continue;

            perror("Error reading input");
            close(fd);
            return 1;
        }

        if (n == 0)
            break;

        if (len >= sizeof(line)) {
            fprintf(stderr, "Input line too long\n");
            close(fd);
            return 1;
        }

        line[len++] = ch;

        if (ch == '\n')
            break;
    }

    int prefix_len = snprintf(
        entry, sizeof(entry), "PID=%ld: ", (long)getpid()
    );

    if (prefix_len < 0 ||
        (size_t)prefix_len + len + 1 > sizeof(entry)) {
        fprintf(stderr, "Log entry too long\n");
        close(fd);
        return 1;
    }

    memcpy(entry + prefix_len, line, len);

    size_t entry_len = (size_t)prefix_len + len;

    if (len == 0 || line[len - 1] != '\n') {
        entry[entry_len++] = '\n';
    }

    size_t written = 0;

    while (written < entry_len) {
        ssize_t n = write(fd, entry + written,
                          entry_len - written);

        if (n == -1) {
            if (errno == EINTR)
                continue;

            perror("Error writing file");
            close(fd);
            return 1;
        }

        if (n == 0) {
            fprintf(stderr, "Write made no progress\n");
            close(fd);
            return 1;
        }

        written += n;
    }

    off_t offset = lseek(fd, 0, SEEK_CUR);

    if (offset == -1) {
        perror("Error getting file offset");
        close(fd);
        return 1;
    }

    printf("Final file offset: %lld bytes\n",
           (long long)offset);

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }

    return 0;
}

