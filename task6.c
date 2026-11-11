#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int write_all(int fd, const char *data, size_t length) {
    size_t total = 0;

    while (total < length) {
        ssize_t n = write(fd, data + total, length - total);

        if (n == -1 && errno == EINTR)
            continue;

        if (n <= 0) {
            if (n == 0)
                errno = EIO;
            return -1;
        }

        total += n;
    }

    return 0;
}

int main(void) {
    int fd = -1;
    char ch;
    char buffer[100];
    char *remainder = NULL;
    int result = 1;

    fd = open("numbers.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    for (int i = 1; i <= 10; i++) {
        char line[16];
        int len = snprintf(line, sizeof(line), "%d\n", i);

        if (write_all(fd, line, (size_t)len) == -1) {
            perror("Error writing numbers");
            goto cleanup;
        }
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        fd = -1;
        goto cleanup;
    }

    fd = open("numbers.txt", O_RDWR);

    if (fd == -1) {
        perror("Error reopening file");
        return 1;
    }

    int newlines = 0;

    while (newlines < 3) {
        ssize_t n = read(fd, &ch, 1);

        if (n == -1 && errno == EINTR)
            continue;

        if (n != 1) {
            fprintf(stderr, "Error finding line 4\n");
            goto cleanup;
        }

        if (ch == '\n')
            newlines++;
    }

    off_t line4_start = lseek(fd, 0, SEEK_CUR);

    if (line4_start == -1) {
        perror("Error getting position");
        goto cleanup;
    }

    do {
        ssize_t n = read(fd, &ch, 1);

        if (n == -1 && errno == EINTR)
            continue;

        if (n != 1) {
            fprintf(stderr, "Error reading line 4\n");
            goto cleanup;
        }
    } while (ch != '\n');

    off_t remainder_start = lseek(fd, 0, SEEK_CUR);
    off_t file_end = lseek(fd, 0, SEEK_END);

    if (remainder_start == -1 || file_end == -1) {
        perror("Error getting file offsets");
        goto cleanup;
    }

    size_t remainder_size = (size_t)(file_end - remainder_start);

    remainder = malloc(remainder_size ? remainder_size : 1);

    if (remainder == NULL) {
        perror("Memory allocation failed");
        goto cleanup;
    }

    if (lseek(fd, remainder_start, SEEK_SET) == -1) {
        perror("Error seeking");
        goto cleanup;
    }

    size_t total = 0;

    while (total < remainder_size) {
        ssize_t n = read(fd, remainder + total,
                         remainder_size - total);

        if (n == -1 && errno == EINTR)
            continue;

        if (n != 1 && n <= 0) {
            fprintf(stderr, "Error reading remainder\n");
            goto cleanup;
        }

        total += n;
    }

    if (lseek(fd, line4_start, SEEK_SET) == -1) {
        perror("Error seeking to line 4");
        goto cleanup;
    }

    if (write_all(fd, "100\n", 4) == -1 ||
        write_all(fd, remainder, remainder_size) == -1) {
        perror("Error overwriting file");
        goto cleanup;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Error rewinding file");
        goto cleanup;
    }

    printf("Final file content:\n");
    fflush(stdout);

    while (1) {
        ssize_t n = read(fd, buffer, sizeof(buffer));

        if (n == -1 && errno == EINTR)
            continue;

        if (n == -1) {
            perror("Error reading final file");
            goto cleanup;
        }

        if (n == 0)
            break;

        if (write_all(STDOUT_FILENO, buffer, (size_t)n) == -1) {
            perror("Error printing file");
            goto cleanup;
        }
    }

    result = 0;

cleanup:
    free(remainder);

    if (fd != -1 && close(fd) == -1) {
        perror("Error closing file");
        result = 1;
    }

    return result;
}
