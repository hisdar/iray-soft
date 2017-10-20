#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
	int fd = open("/dev/ttyS1", O_RDWR);
	if (fd <= 0)
	{
		printf("open /dev/ttyS1 fail\n");
		return -1;
	}

	char cmd[] = {0xAA, 0x05, 0x01, 0xA3, 0x01, 0x00, 0x54, 0xEB, 0xAA};
	int len = sizeof(cmd);
	int ret = write(fd, cmd, len);
	if (ret < len)
	{
		printf("write cmd fail, ret=%d, len=%d\n", ret, len);
		return -1;
	}

	char buf = 0;
	int read_count = 0;
	while (1) {
		ret = read(fd, &buf, 1);
		if (ret < 0)
		{
			break;
		}

		read_count += 1;
		printf("0x%02x \n", buf);

		if (read_count % 8 == 0)
		{
			printf("\n");
		}
	}
	
	return 0;
}

