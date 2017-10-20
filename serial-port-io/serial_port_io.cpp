
#include <stdio.h>		/*标准输入输出定义*/
#include <stdlib.h>		/*标准函数库定义*/
#include <unistd.h>		/*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>		/*文件控制定义*/
#include <termios.h>		/*PPSIX 终端控制定义*/
#include <errno.h>		/*错误号定义*/
#include <string.h>

#define FALSE		(-1)
#define TRUE		(0)
#define ERR_FILE	(-1)

#define print_e(format, args...) \
	{\
		printf("[Error]");\
		printf(format, ##args);\
	}

#define print_i(format, args...) \
	{\
		printf("[Infor]");\
		printf(format, ##args);\
	}

#define print_d(format, args...) \
	{\
		printf("[Debug]");\
		printf(format, ##args);\
	}

int open_device(char* device_path)
{
	int ret = 0;
	int fd = 0;

	if (NULL == device_path)
	{
		print_e("device path is null\n");
		return ERR_FILE;
	}
	
	fd = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY);
	if (ERR_FILE == fd)
	{
		print_e("Can't Open Serial Port, path=%s\n", device_path);
		return ERR_FILE;
	}
	
	// set serial port to non-blocking
	ret = fcntl(fd, F_SETFL, 0);
	if (ret < 0)
	{
		print_e("set serial port to non-blocking fail, ret=%d!\n", ret);
		return ERR_FILE;
	}
	
	// print_i("set serial port to non-blocking success\n");

	// test the device is a terminal device
	if (0 == isatty(STDIN_FILENO))
	{
		print_e("standard input is not a terminal device\n");
		return ERR_FILE;
	}
		
	// print_i("serial port fd is:%d\n", fd);

	return fd;
}	

void close_device(int fd)
{
	close(fd);
}

void set_serial_port_flow_ctrl(struct termios *op, int flow_ctrl)
{
	if (NULL == op)
	{
		return;
	}

	switch(flow_ctrl)
	{
		case 0:  // do not use flow control
			op->c_cflag &= ~CRTSCTS;
			break;

		case 1:  // use hardware flow control
			op->c_cflag |= CRTSCTS;
			break;

		case 2:  // use software flow control
			op->c_cflag |= IXON | IXOFF | IXANY;
			break;

		default:
			break;
	}
}

void set_serial_port_data_size(struct termios *op, int data_size)
{
	if (NULL == op)
	{
		return;
	}

	op->c_cflag &= ~CSIZE;
	switch (data_size)
	{
		case 5:
			op->c_cflag |= CS5;
			break;
		case 6:
			op->c_cflag |= CS6;
			break;
		case 7:
			op->c_cflag |= CS7;
			break;
		case 8:
			op->c_cflag |= CS8;
			break;
		default:
			print_e("unsupported data size:%d\n", data_size);
	}
	
}

void set_serial_port_verify_bits(struct termios *op, char parity)
{
	if (NULL == op)
	{
		return;
	}

	switch (parity)
	{
		case 'n':
		case 'N': // no verify bits
			op->c_cflag &= ~PARENB;
			op->c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O': // odd verify
			op->c_cflag |= (PARODD | PARENB);
			op->c_iflag |= INPCK;
			break;
		case 'e':
		case 'E': // even verify
			op->c_cflag |= PARENB;
			op->c_cflag &= ~PARODD;
			op->c_iflag |= INPCK;
			break;
		case 's':
		case 'S': // set to space
			op->c_cflag &= ~PARENB;
			op->c_cflag &= ~CSTOPB;
			break;
		default:
			print_e("unsupported parity:%d\n", parity);
	}
}

void set_serial_port_stop_bits(struct termios *op, int stop_bits)
{
	if (NULL == op)
	{
		return;
	}

	switch (stop_bits)
	{
		case 1:
			op->c_cflag &= ~CSTOPB;
			break;

		case 2:
			op->c_cflag |= CSTOPB;
			break;

		default:
			print_e("unsupported stop bits:%d\n", stop_bits);
	}
}

int set_serial_port_attr(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{
	int ret = 0;

	struct termios op;
	   
	/* get serial port attr */
	ret = tcgetattr(fd, &op);
	if (ret != 0)
	{
		print_e("get serial port attr fail, ret=%d\n", ret);
		return ret;
	}

	// set speed, ensure the program will not 
	cfsetispeed(&op, speed);
	cfsetospeed(&op, speed);

	//  set control model, occupy serial port
	op.c_cflag |= CLOCAL;

	// set control model, so that program can read data from serial port
	op.c_cflag |= CREAD;

	// set flow control
	set_serial_port_flow_ctrl(&op, flow_ctrl);
	
	// set data size
	set_serial_port_data_size(&op, databits);
	
	// set verify bit
	set_serial_port_verify_bits(&op, parity);

	// set stop bit
	set_serial_port_stop_bits(&op, stopbits);

	// set out put model to no handle
	op.c_oflag &= ~OPOST;

	op.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// set mini wait time and mini read bytes
	op.c_cc[VMIN] = 0;
	op.c_cc[VTIME] = 0;

	// clear data buffer
	tcflush(fd, TCIFLUSH);

	// set attr to serial port
	ret = tcsetattr(fd, TCSANOW, &op);
	if (ret != 0)
	{
		print_e("set serial port attr fail, ret=%d!\n", ret);
		return ERR_FILE;
	}
	
	return TRUE;
}	

int receive(int fd, char *rcv_buf, int data_len)
{
	int len, fs_sel;
	fd_set fs_read;
	struct timeval time;
	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);

	time.tv_sec = 10;
	time.tv_usec = 0;

	// 使用 select 实现串口的多路通信
	fs_sel = select(fd + 1, &fs_read, NULL,NULL, &time);
	if (fs_sel)
	{
		len = read(fd, rcv_buf, data_len);
		return len;
	}
	else
	{
		return FALSE;
	}
}

int send(int fd, char *data,int data_len)
{
	int len = 0;

	len = write(fd, data, data_len);
	if (len == data_len)
	{
		return len;
	}
	else
	{
		tcflush(fd, TCOFLUSH);
		return ERR_FILE;
	}
}

int send_data(char *dev_path, char *src_data, int data_len)
{
	int fd = 0;
	int ret = 0;
	
	fd = open_device(dev_path);

	ret = set_serial_port_attr(fd, B115200, 0, 8, 1, 'N');

	ret = send(fd, src_data, data_len);
	if (ret) {}
	close_device(fd);
	
	return TRUE;
}

int receive_data(char *dev_path)
{
	int fd = 0;
	int ret = 0;
	char rcv[512] = {0};
	char end_flag[2] = {0};
	int data_counter = 0;
	
	if (dev_path == NULL)
	{
		return ERR_FILE;
	}
	
	fd = open_device(dev_path);

	ret = set_serial_port_attr(fd, B115200, 0, 8, 1, 'N');

	while(1)
	{
		ret = receive(fd, rcv, 512);
		if (ret > 0)
		{
			for (int i = 0; i < ret; i++)
			{
				data_counter += 1;
				printf("%02x ", rcv[i]);


				end_flag[0] = end_flag[1];
				end_flag[1] = rcv[i];

				if (end_flag[0] == 0xEB && end_flag[1] == 0xAA)
				{
					printf("\n");
				}
			}
		}

		if (ret <= 0)
		{
			sleep(1);
		}
	}

	close_device(fd);

	return TRUE;
}

void print_help_info()
{
	printf("\n******** serial port io tool help message ********\n");
	printf("\t-h/-help: this message\n");
	printf("\t-r: receive data: ./spio /dev/ttyS0 -r \n");
	printf("\t-s: send    data: ./spio /dev/ttyS0 -s \"01 02 03\"\n");
	printf("\n");
}

int parse_input_data(const char *data, int data_len, char *out, int out_len)
{
	int ptr_num_idx = 0;
	int out_put_data_idx = 0;
	char ptr_num_buf[3]  = {0};

	unsigned int hex_data = 0;
		
	if (data == NULL || data_len <= 0)
	{
		print_e("parameter error, data_len=%d\n", data_len);
		return ERR_FILE;
	}

	// check data
	for (int i = 0; i < data_len; i++)
	{
		if ( (data[i] >= '0' && data[i] <= '9')
			|| (data[i] >= 'A' && data[i] <= 'F')
			|| (data[i] >= 'f' && data[i] <= 'f')
			|| (data[i] == ' ')) {
			continue;
		} else {

			print_e("error data:[%c], at[%d]\n", data[i], i);
			return ERR_FILE;
		}
	}

	// parse data
	for (int i = 0; i < data_len; i++)
	{
		// found the end flag
		if ((data[i] == ' '))
		{
			if (ptr_num_idx > 0 && out_put_data_idx < out_len)
			{
				sscanf(ptr_num_buf, "%x", &hex_data);
				out[out_put_data_idx] = (char)(int)hex_data;
				out_put_data_idx += 1;
			}
		
			ptr_num_idx = 0;
			memset(ptr_num_buf, 0, sizeof(ptr_num_buf));
			continue;
		}

		if (ptr_num_idx >= 2)
		{
			print_e("error data at[%d], a hex data only have 2 bytes\n", i);
			return ERR_FILE;
		}

		ptr_num_buf[ptr_num_idx] = data[i];
		ptr_num_idx += 1;
	}

	if (ptr_num_idx > 0 && out_put_data_idx < out_len)
	{
		sscanf(ptr_num_buf, "%x", &hex_data);
		out[out_put_data_idx] = (char)(hex_data & 0xFF);
		out[out_put_data_idx] &= 0xFF;
		out_put_data_idx += 1;
	}

	return out_put_data_idx;
}

int main(int argc, char **argv)
{
	int len = 0;
	char send_buf[32] = {0};

	switch (argc)
	{
		case 3:
			// receive data
			receive_data(argv[1]);

			break;
		case 4:
			// send data
			len = parse_input_data(argv[3], strlen(argv[3]), send_buf, 20);
			if (len > 0)
			{
				send_data(argv[1], send_buf, len);
			}

			break;
		case 2:
		default:
			// print help information
			print_help_info();
			break;
	}

	return 0;
}

