#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/root_cmd_pipe"
#define MAX_CMD_LEN 1024

int main(int argc, char *argv[]) {
    int fd;
    char cmd[MAX_CMD_LEN];

    if (argc < 2) {
        fprintf(stderr, "用法: %s \"命令\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    snprintf(cmd, MAX_CMD_LEN, "%s\n", argv[1]);

    fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (write(fd, cmd, strlen(cmd)) < 0) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    return 0;
}
