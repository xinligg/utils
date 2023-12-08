#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <linux/fs.h>

int main(int argc, char **argv)
{
        int fd;
        unsigned long long bytes;
	char *device;

	device = argv[1];

	fd = open(device, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
                printf("cannot open %s", device);
                return -1;
        }

        if (blkdev_get_size(fd, &bytes) == 0) {
                printf("%s %lld\n",device, bytes);
        }

        close(fd);
	return 0;
}

/* get size in bytes */
int blkdev_get_size(int fd, unsigned long long *bytes)
{
#ifdef BLKGETSIZE64
        if (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
                return 0;
#endif
}
