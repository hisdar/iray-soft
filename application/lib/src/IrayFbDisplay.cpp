#include <unistd.h>
#include <string.h>
#include <linux/fb.h>
#include <IrayFbDisplay.h>
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
#include <linux/fb.h>
#include <stdio.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

IrayFbDisplay::IrayFbDisplay()
{

}


IrayFbDisplay::~IrayFbDisplay()
{

}
unsigned int addr = 0;
unsigned int buffersize = 0;

// FBDEV Driver
int get_fix_screen_info(char * fb_dev)
{
	int ret = 0;

	int fd = 0;

	/* Open a graphics Display logical channel in blocking mode */
	fd = open (fb_dev, O_RDWR);
	if (fd == -1) {
	    perror("failed to open display device\n");
	    return -1;
	}

	
	/* Getting fix screen information */
	struct fb_fix_screeninfo fix;
	ret = ioctl(fd, FBIOGET_FSCREENINFO, &fix);
	if(ret < 0) {
		printf("Cannot get fix screen information\n");
		close(fd);
		exit(0);
	}

	printf("Line length = %d\n",fix.line_length);
	printf("Physical Address = %x\n",fix.smem_start);
	printf("Buffer Length = %d\n",fix.smem_len);


	/* Getting fix screen information */
	struct fb_var_screeninfo var;
	ret = ioctl(fd, FBIOGET_VSCREENINFO, &var);
	if(ret < 0) {
		printf("Cannot get variable screen information\n");
		close(fd);
		exit(0);
	}
	printf("Resolution = %dx%d\n",var.xres, var.yres);
	printf("bites per pixel = %d\n",var.bits_per_pixel);

	/* addr hold the user space address */
	
	/* Get the fix screen info */
	/* Get the variable screen information */
	buffersize = fix.line_length * var.yres;
	addr = (unsigned int)mmap(NULL, buffersize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	/* buffer.m.offset is same as returned from VIDIOC_QUERYBUF */

	/* Set the rotation through ioctl. */
	/* Get the Variable screen info through "FBIOGET_VSCREENINFO" */
	var.rotate = 1; /* To set rotation angle to 90 degree */
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &var)<0) {
		perror("Error:FBIOPUT_VSCREENINFO\n");
		close(fd);
		exit(4);
	}

	
	//struct fb_var_screeninfo var;
	/* Get variable screen information. Variable screen information
	* gives information like size of the image, bites per pixel,
	* virtual size of the image etc. */
	ret = ioctl(fd, FBIOGET_VSCREENINFO, &var);
	if (ret < 0) {
		perror("Error reading variable information.\n");
		close(fd);
		exit(3);
	}
	/* Set bits per pixel and offsets*/
	var.red.length= 8;
	var.green.length = 8;
	var.blue.length = 8;
	var.transp.length= 8;
	var.transp.offset = 24;
	var.red.offset = 16;
	var.green.offset =8;
	var.blue.offset = 0;
	var.bits_per_pixel = 32;
	var.left_margin = 640;
	var.right_margin = 640;
	var.upper_margin = 252;
	var.lower_margin = 252;
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &var)<0) {
		perror("Error:FBIOPUT_VSCREENINFO\n");
		close(fd);
		exit(4);
	}

	//struct fb_var_screeninfo var;
	/*var.bits_per_pixel = 16;
	var.red.length = var.green.length = var.blue.length = 4;
	var.red.offset = 8;
	var.green.offset = 4;
	var.blue.offset = 0;
	ret = ioctl(fd, FBIOPUT_VSCREENINFO, &var);
	if (ret < 0) {
		perror("FBIOPUT_VSCREENINFO\n");
		close(fd);
		exit(0);
	}*/


	//while(1) {
		//memset((char *)addr, 0x00, buffersize / 2);
		//memset((char *)(addr + buffersize / 2), 0x255, buffersize / 2);
	//}

	/* Closing of channels */
	//close (fd);

}


int IrayFbDisplay::receiveFrame(IrayCameraData *frameData)
{
	int ret= 0;
	struct timeval tv;

	if (addr == 0) {
		get_fix_screen_info("/dev/fb0");
	}

	int len = frameData->getLength() > buffersize ? buffersize : frameData->getLength();

	memcpy((char *)addr, frameData->getAddr(), len);
	
	return 0;
}

