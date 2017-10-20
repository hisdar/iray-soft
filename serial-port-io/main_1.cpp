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
    
    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);    //Ĭ��Ϊ��������ʽ
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
	
	// 1.����ѡ����CLOCAL��CREAD
	// CLOCAL��CREAD�ֱ����ڱ������Ӻͽ���ʹ�ܣ�ͨ��λ����ķ�ʽ����������ѡ�
	opt.c_cflag |= (CLOCAL | CREAD);

	// 2.���ò�����
    cfsetispeed(&opt, B115200);
    cfsetospeed(&opt, B115200);

	// 3.�����ַ���С
	// һ����ȥ������λ�е�λ���룬�����°�Ҫ�����á�
	opt.c_cflag &= ~CSIZE; //mask the character size bits
	opt.c_cflag |= CS8;

	// 4.����У��λ:��У��
	opt.c_cflag &= ~PARENB;
	opt.c_cflag &= ~INPCK;

	// 5.����ֹͣλ
	// ��ֹͣλΪ1�������CSTOPB����ֹͣλΪ0���򼤻�CSTOPB
	opt.c_cflag &= ~CSTOPB;

	// ����һ�����ã��������óɲ���Ҫ�س���ֱ�ӷ���
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// ��������
	opt.c_oflag &= ~OPOST;
	
	// ������������ NL/CR��ʱ�� ת���� CR+NL
	opt.c_oflag &= ~(ONLCR | OCRNL);
	opt.c_iflag &= ~(ICRNL | INLCR | IGNCR);
	opt.c_iflag &= ~ISTRIP;

	// ����˵��
	// IXON	�������ʱ��XON/XOFF�����п���
	// IXANY	�����κ��ַ�������ֹͣ�����
	// IXOFF	��������ʱ��XON/XOFF�����п���
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);    //��ӵ�

	// ���õȴ�ʱ��������ַ�
	// �ڶԽ����ַ��͵ȴ�ʱ��û���ر�Ҫ�������£����Խ�������Ϊ0��
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
