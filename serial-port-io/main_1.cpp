#include     <stdio.h>
#include     <stdlib.h> 
#include     <unistd.h>  
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h> 
#include     <termios.h>
#include     <errno.h>
   
main()
{
    int fd;
    int i;
	int ret;
    int len;
    int n = 0;
    struct termios opt; 
    
    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);    //默认为阻塞读方式
    if (fd == -1)
    {
        perror("open serial 0\n");
        exit(0);
    }

    ret = tcgetattr(fd, &opt);      
	if (ret < 0)
	{
		printf("get tc attr fail, ret=%d\n", ret);
	}

	printf("%s:%d\n%s:%d\n%s:%d\n%s:%d\n%s:%d\n%s:%d\n%s:%d\n%s:%d\n",
		"i-speed", cfgetispeed(&opt),
		"o-speed", cfgetospeed(&opt),
		"v-time", opt.c_cc[VTIME],
		"v-mini", opt.c_cc[VTIME],
		"c_cflag", opt.c_cflag,
		"c_lflag", opt.c_lflag,
		"c_oflag", opt.c_oflag,
		"c_iflag", opt.c_iflag);
	
	// 1.激活选项有CLOCAL和CREAD
	// CLOCAL和CREAD分别用于本地连接和接受使能，通过位掩码的方式激活这两个选项。
	opt.c_cflag |= (CLOCAL | CREAD);

	// 2.设置波特率
    cfsetispeed(&opt, B115200);
    cfsetospeed(&opt, B115200);

	// 3.设置字符大小
	// 一般先去除数据位中的位掩码，再重新按要求设置。
	opt.c_cflag &= ~CSIZE; //mask the character size bits
	opt.c_cflag |= CS8;

	// 4.设置校验位:无校验
	opt.c_cflag &= ~PARENB;
	opt.c_cflag &= ~INPCK;

	// 5.设置停止位
	// 若停止位为1，则清除CSTOPB，若停止位为0，则激活CSTOPB
	opt.c_cflag &= ~CSTOPB;

	// 以下一行设置，用于设置成不需要回车键直接发送
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// 处理后输出
	opt.c_oflag &= ~OPOST;
	
	// 单独输入输入 NL/CR的时候 转换成 CR+NL
	opt.c_oflag &= ~(ONLCR | OCRNL);
	opt.c_iflag &= ~(ICRNL | INLCR | IGNCR);
	opt.c_iflag &= ~ISTRIP;

	// 参数说明
	// IXON	允许输出时对XON/XOFF流进行控制
	// IXANY	输入任何字符将重启停止的输出
	// IXOFF	允许输入时对XON/XOFF流进行控制
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);    //添加的

	// 设置等待时间和最少字符
	// 在对接收字符和等待时间没有特别要求的情况下，可以将其设置为0：
	opt.c_cc[VTIME] = 0;
	opt.c_cc[VMIN] = 0;
	
    tcflush(fd, TCIOFLUSH);
    if (tcsetattr(fd, TCSANOW, &opt) != 0 )
    {     
       perror("tcsetattr error");
       return -1;
    }

	printf("configure complete\n");

	// char cmd[] = {0xAA, 0x05, 0x01, 0xA3, 0x01, 0x00, 0x54, 0xEB, 0xAA};
//	char cmd[] = {0xAA, 0x06, 0x01, 0x5D, 0x02, 0x04, 0x00, 0x14, 0xEB, 0xAA};
	char cmd[] = {0xAA, 0x05, 0x00, 0x30, 0x01, 0x01, 0xE1, 0xEB, 0xAA};
	len = sizeof(cmd);
	ret = write(fd, cmd, len);
	if (ret < len)
	{
		printf("write cmd fail, ret=%d, len=%d\n", ret, len);
		return -1;
	}

	char buf[64] = {0};
	int read_count = 0;
	while (1) {
		ret = read(fd, buf, 64);
		if (ret <= 0)
		{
			sleep(1);
			continue;
		}

		for (int i = 0; i < ret; i++)
		{
			printf("%02x ", buf[i]);

			read_count += 1;
			if (read_count % 8 == 0) printf("\n");
		}
	}
}
