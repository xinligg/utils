#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#define FIFO_PATH "/tmp/root_cmd_pipe"
#define MAX_CMD_LEN 1024

void cleanup() {
    unlink(FIFO_PATH);
}

void sig_handler(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        cleanup();
        exit(0);
    }
}

int main() {
    FILE *fp;
    int fd;
    char cmd[MAX_CMD_LEN];

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    atexit(cleanup);

    mkfifo(FIFO_PATH, 0666);
    chmod(FIFO_PATH, 0666);

    fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    fp = fdopen(fd, "r");
    if (!fp) {
        perror("fdopen");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Root程序等待命令...\n");
    while (fgets(cmd, MAX_CMD_LEN, fp) != NULL) {
        cmd[strcspn(cmd, "\n")] = '\0';
        printf("执行命令: %s\n", cmd);
        int ret = system(cmd);
        if (ret != 0) {
            fprintf(stderr, "命令执行失败，返回值：%d\n", ret);
        }
    }

    fclose(fp);
    return 0;
}
