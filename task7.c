#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 4096

ssize_t read_chunk(int fd, unsigned char *buffer, size_t size) {
    size_t total = 0;

    while (total < size) {
        ssize_t n = read(fd, buffer + total, size - total);

        if (n == -1) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            break;

        total += n;
    }

    return (ssize_t)total;
}

int main(void) {
    char path1[1024];
    char path2[1024];

    unsigned char buffer1[BUFFER_SIZE];
    unsigned char buffer2[BUFFER_SIZE];

    int fd1 = -1;
    int fd2 = -1;
    int result = 1;

    unsigned long long position = 0;

    printf("Enter first file path: ");
    fflush(stdout);

    if (fgets(path1, sizeof(path1), stdin) == NULL) {
        fprintf(stderr, "Error reading first path\n");
        return 1;
    }

    printf("Enter second file path: ");
    fflush(stdout);

    if (fgets(path2, sizeof(path2), stdin) == NULL) {
        fprintf(stderr, "Error reading second path\n");
        return 1;
    }

    path1[strcspn(path1, "\n")] = '\0';
    path2[strcspn(path2, "\n")] = '\0';

    fd1 = open(path1, O_RDONLY);

    if (fd1 == -1) {
        perror("Error opening first file");
        goto cleanup;
    }

    fd2 = open(path2, O_RDONLY);

    if (fd2 == -1) {
        perror("Error opening second file");
        goto cleanup;
    }

    while (1) {
        ssize_t n1 = read_chunk(fd1, buffer1, BUFFER_SIZE);

        if (n1 == -1) {
            perror("Error reading first file");
            goto cleanup;
        }

        ssize_t n2 = read_chunk(fd2, buffer2, BUFFER_SIZE);

        if (n2 == -1) {
            perror("Error reading second file");
            goto cleanup;
        }

        size_t common = (n1 < n2) ? (size_t)n1 : (size_t)n2;

        for (size_t i = 0; i < common; i++) {
            if (buffer1[i] != buffer2[i]) {
                printf("Files differ at byte %llu\n",
                       position + i);
                goto cleanup;
            }
        }

        if (n1 != n2) {
            printf("Files differ at byte %llu\n",
                   position + common);
            goto cleanup;
        }

        if (n1 == 0) {
            printf("Files are identical\n");
            result = 0;
            break;
        }

        position += (unsigned long long)n1;
    }

cleanup:
    if (fd1 != -1 && close(fd1) == -1) {
        perror("Error closing first file");
        result = 1;
    }

    if (fd2 != -1 && close(fd2) == -1) {
        perror("Error closing second file");
        result = 1;
    }

    return result;
}
