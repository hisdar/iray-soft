#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include "IrayCamera.h"
#include <NetRecevier.h>
#include <IrayImageWriter.h>
#include <IrayFrameRate.h>

int save_to_hex_file(char *path, int count);
int save_to_raw_file(char *path, int count);

void print_help_info()
{
	printf("net camera help information\n");
	printf("\t-h/-help : show this message\n");
	
	printf("\t-net [ipaddress] [port] : connect to server and send picture to server\n");

	printf("\t-f [filepath] [filetype] <count> : save image picture to file\n");
	printf("\t\t [filepath] : like: ./image\n");
	printf("\t\t [filetype] : should be: -hex/-raw\n");
	printf("\t\t <count>    : default value is 1\n");

	printf("\t-fr : statistics frame rate\n");
	printf("\t-show-frame-info [count]: show frame info\n");
	
}


int start_net_camera(char *ip_addr, char *src_port)
{
	int ret = 0;
	int port = 0;
	NetReceiver netRecviver;
	
	// init net
	ret = sscanf(src_port, "%d", &port);
	if (ret != 1) {
		printf("parse port value fail, ret=%d, src=%s\n", ret, src_port);
		return -EPARAME;
	}

	ret = netRecviver.initSocket(ip_addr, port);
	if (ret != SUCCESS) {
		printf("init_socket fail, ret=%d, ip=%s, port=%d\n", ret, ip_addr, port);
		return ret;
	}

	IrayCamera camera;
	camera.setFrameReceiver(&netRecviver);
	camera.startCapture(0);  // no stop

	return 0;
}

int save_to_file(int argc, char* argv[])
{
	int ret = 0;
	int count = 0;
	char *file_name = NULL;
	char *file_type = NULL;
	char *ptr_count = NULL;

	if (argc < 4 || argc > 5) {
		return -EPARAME;
	}
	
	file_name = argv[2];
	file_type = argv[3];

	if (argc == 5) {
		ptr_count = argv[4];
		ret = sscanf(ptr_count, "%d", &count);
		if (ret != 1) {
			printf("parse count value fail, ret=%d, src=%s\n", ret, ptr_count);
			return -EPARAME;
		}
	} else {
		count = 1;
	}

	if (strcmp(file_type, "-raw") == 0) {
		return save_to_raw_file(file_name, count);
	} else if (strcmp(file_type, "-hex") == 0) {
		return save_to_hex_file(file_name, count);
	} else {
		printf("unhandled file_type[%s]\n", file_type);
		return -EPARAME;
	}

	return 0;
}

int statistics_frame_rate()
{
	IrayCamera camera;
	IrayFrameRate frameRate(DEFAULT_BUFFER_SIZE, 1);

	camera.setFrameReceiver(&frameRate);
	camera.startCapture(0);

	return 0;
}

int show_frame_info(char * ptr_count)
{
	int ret = 0;
	int count = 0;
	IrayCamera camera;

	ret = sscanf(ptr_count, "%d", &count);
	if (ret != 1) {
		printf("parse count value fail, ret=%d, src=%s\n", ret, ptr_count);
		return -EPARAME;
	}


	camera.startCapture(count, 1);
	return 0;
}


int save_to_raw_file(char *path, int count)
{
	IrayCamera camera;
	IrayImageWriter imageWriter(path, IRAY_IMAGE_FILE_TYPE_RAW);

	camera.setFrameReceiver(&imageWriter);
	camera.startCapture(count);
	
	return 0;
}

int save_to_hex_file(char *path, int count)
{
	IrayCamera camera;
	IrayImageWriter imageWriter(path, IRAY_IMAGE_FILE_TYPE_HEX);

	camera.setFrameReceiver(&imageWriter);
	camera.startCapture(count);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		print_help_info();
	}

	if (strcmp(argv[1], "-h") == 0) {
		print_help_info();
	} else if (strcmp(argv[1], "--help") == 0) {
		print_help_info();
	} else if (strcmp(argv[1], "-net") == 0) {
		start_net_camera(argv[2], argv[3]);
	} else if (strcmp(argv[1], "-f") == 0) {
		save_to_file(argc, argv);
	} else if (strcmp(argv[1], "-fr") == 0) {
		statistics_frame_rate();
	} else if (strcmp(argv[1], "-show-frame-info") == 0) {
		show_frame_info(argv[2]);
	} else {
		print_help_info();
	}

	return 0;
}

