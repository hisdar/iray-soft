#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <memory.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#define LOOPCOUNT 500

int main(int argc, char *argv[])
{
	int i = 0;
	int j = 0;
	int fd = 0;
	int ret = 0;
	int numbuffers = 10;

	/* Open a video Display logical channel in blocking mode */
	fd = open ("/dev/video0", O_RDWR);
	if (fd == -1) {
		perror("Failed to open display device\n");
		return -1;
	}

	/* structure to store buffer request parameters */
	struct v4l2_requestbuffers reqbuf;
	reqbuf.count = numbuffers;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fd , VIDIOC_REQBUFS, &reqbuf);
	if(ret < 0) {
		printf("cannot allocate memory\n");
		close(fd);
		return -1;
	}

	/* allocate buffer by VIDIOC_REQBUFS */
	/* structure to query the physical address
	of allocated buffer */
	struct v4l2_buffer buffer;
	/* buffer index for querying -0 */
	buffer.index = 0;
	buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buffer.memory = V4L2_MEMORY_MMAP;
	if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) < 0) {
		printf("buffer query error.\n");
		close(fd);
		exit(-1);
	}
	/*The buffer.m.offset will contain the physical address returned from driver*/

	/* allocate buffer by VIDIOC_REQBUFS */
	/* query the buffer using VIDIOC_QUERYBUF */
	/* addr hold the user space address */
	char * addr;
	addr = (char *)mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);
	/* buffer.m.offset is same as returned from VIDIOC_QUERYBUF */


	/* Initially fill the buffer */
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	struct v4l2_format fmt;
	/* Fill the buffers with the image */
	/* Enqueue buffers */
	for (i = 0; i < req.count; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.index = i;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("VIDIOC_QBUF\n");
			for (j = 0; j < req.count; j++){
				/* Unmap all the buffers if call fails */
				exit(0);
			}
			printf("VIDIOC_QBUF = %d\n",i);
		}
	}
	/* Start streaming */
	int a = 0;
	ret = ioctl(fd, VIDIOC_STREAMON, &a);
	if (ret < 0) {
		perror("VIDIOC_STREAMON\n");
		for (i = 0; i < req.count; i++)
			/* Unmap all the buffers if call fails */
		exit(0);
	}
	/* loop for streaming with 500 Frames*/
	for(i = 0 ;i < LOOPCOUNT ;i ++) {
		ret = ioctl(fd, VIDIOC_DQBUF, &buf);
		if(ret < 0){
			perror("VIDIOC_DQBUF\n");
			for (j = 0; j < req.count; j++){
				/* Unmap all the buffers if call fails */
			}
			exit(0);
		}
		/* Fill the buffer with new data
		fill(buff_info[buf.index].start, fmt.fmt.pix.width, fmt.fmt.pix.height,0);
		/Queue the buffer again */
		ret = ioctl(fd, VIDIOC_QBUF, &buf);
		if(ret < 0){
			perror("VIDIOC_QBUF\n");
			for (j = 0; j < req.count; j++){
				 /* Unmap all the buffers if call fails */
			}
			exit(0);
		}
	}
	/* Streaming off */
	ret = ioctl(fd, VIDIOC_STREAMOFF, &a);
	if (ret < 0) {
		perror("VIDIOC_STREAMOFF\n");
		for (i = 0; i < req.count; i++){
			/* Unmap all the buffers if call fails */
		}
		exit(0);
	}


	/* Closing of channel */
	close (fd);

	return 0;
}
	

