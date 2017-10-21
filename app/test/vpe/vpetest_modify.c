/*
 *  Copyright (c) 2012-2013, Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  ============================================================================
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>

static int debug = 0;
#define dprintf(fmt, arg...) if (debug) {printf(fmt, ## arg); fflush(stdout);}

#define pexit(str) { \
	perror(str); \
	exit(1); \
}

#define V4L2_CID_TRANS_NUM_BUFS         (V4L2_CID_USER_TI_VPE_BASE + 0)

/**<	Input  file descriptor				*/
int	fin = -1;

/**<	Output file descriptor				*/
int	fout = -1;

/**<	File descriptor for device			*/
int	fd   = -1;

int describeFormat(
	char	*format,
	int	width,
	int	height,
	int	*size,
	int	*fourcc,
	int	*num_planes,
	enum v4l2_colorspace *clrspc)
{
	*size   = -1;
	*fourcc = -1;
	if (strcmp(format, "rgb24") == 0) {
		*fourcc = V4L2_PIX_FMT_RGB24;
		*size = height * width * 3;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SRGB;

	} else if (strcmp(format, "bgr24") == 0) {
		*fourcc = V4L2_PIX_FMT_BGR24;
		*size = height * width * 3;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SRGB;

	} else if (strcmp(format, "argb32") == 0) {
		*fourcc = V4L2_PIX_FMT_RGB32;
		*size = height * width * 4;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SRGB;

	} else if (strcmp(format, "abgr32") == 0) {
		*fourcc = V4L2_PIX_FMT_BGR32;
		*size = height * width * 4;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SRGB;

	} else if (strcmp(format, "rgb565") == 0) {
		*fourcc = V4L2_PIX_FMT_RGB565;
		*size = height * width * 2;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SRGB;

	} else if (strcmp(format, "yuv444") == 0) {
		*fourcc = V4L2_PIX_FMT_YUV444;
		*size = height * width * 3;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "yvyu") == 0) {
		*fourcc = V4L2_PIX_FMT_YVYU;
		*size = height * width * 2;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "yuyv") == 0) {
		*fourcc = V4L2_PIX_FMT_YUYV;
		*size = height * width * 2;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "uyvy") == 0) {
		*fourcc = V4L2_PIX_FMT_UYVY;
		*size = height * width * 2;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "vyuy") == 0) {
		*fourcc = V4L2_PIX_FMT_VYUY;
		*size = height * width * 2;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nv16") == 0) {
		*fourcc = V4L2_PIX_FMT_NV16;
		*size = height * width * 2;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nv61") == 0) {
		*fourcc = V4L2_PIX_FMT_NV61;
		*size = height * width * 2;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nv12s") == 0) {
		*fourcc = V4L2_PIX_FMT_NV12;
		*size = height * width * 1.5;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nv21s") == 0) {
		*fourcc = V4L2_PIX_FMT_NV21;
		*size = height * width * 1.5;
		*num_planes = 1;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nv12") == 0) {
		*fourcc = V4L2_PIX_FMT_NV12;
		*size = height * width * 1.5;
		*num_planes = 2;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nv21") == 0) {
		*fourcc = V4L2_PIX_FMT_NV21;
		*size = height * width * 1.5;
		*num_planes = 2;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nm12") == 0) {
		*fourcc = V4L2_PIX_FMT_NV12M;
		*size = height * width * 1.5;
		*num_planes = 2;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else if (strcmp(format, "nm21") == 0) {
		*fourcc = V4L2_PIX_FMT_NV21M;
		*size = height * width * 1.5;
		*num_planes = 2;
		*clrspc = V4L2_COLORSPACE_SMPTE170M;

	} else {
		return 0;
	}

	return 1;
}

struct v4l2_format dst_fmt;
struct v4l2_format src_fmt;

int	srcSize    = 0;
int srcSize_uv = 0;

int dstSize   = 0;
int dstSize_uv = 0;

int dstWidth  = 0;
int dstHeight = 0;

int srcWidth = 0;
int srcHeight  = 0;

int	dst_num_planes;
int src_num_planes;

int interlace = 0;

int dst_numbuf = 6;
int src_numbuf = 6;

enum v4l2_colorspace dst_colorspace;
enum v4l2_colorspace src_colorspace;
struct v4l2_selection selection;


int dstFourcc = 0;

int dst_s_fmt()
{
	int ret = 0;
	bzero(&dst_fmt, sizeof(dst_fmt));
	dst_fmt.type				   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	dst_fmt.fmt.pix_mp.width	   = dstWidth;
	dst_fmt.fmt.pix_mp.height	   = dstHeight;
	dst_fmt.fmt.pix_mp.pixelformat = dstFourcc;
	dst_fmt.fmt.pix_mp.colorspace  = dst_colorspace;
	dst_fmt.fmt.pix_mp.num_planes  = dst_num_planes;
	dst_fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;

	ret = ioctl(fd, VIDIOC_S_FMT, &dst_fmt);
	if (ret < 0) {
		pexit("Cant set color format\n");
	} else {
		dstSize = dst_fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
		dstSize_uv = dst_fmt.fmt.pix_mp.plane_fmt[1].sizeimage;
	}

	return ret;
}

int src_s_fmt()
{
	int ret = 0;
	bzero(&src_fmt, sizeof(src_fmt));
	src_fmt.type		= V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	src_fmt.fmt.pix_mp.width	= srcWidth;
	src_fmt.fmt.pix_mp.height	= srcHeight;
	src_fmt.fmt.pix_mp.pixelformat = srcFourcc;
	src_fmt.fmt.pix_mp.colorspace = src_colorspace;
	src_fmt.fmt.pix_mp.num_planes = src_num_planes;

	if (interlace == 1)
		src_fmt.fmt.pix_mp.field = V4L2_FIELD_ALTERNATE;
	else if (interlace == 2)
		src_fmt.fmt.pix_mp.field = V4L2_FIELD_SEQ_TB;
	
	ret = ioctl(fd, VIDIOC_S_FMT, &src_fmt);
	if (ret < 0) {
		pexit("Cant set color format\n");
	} else {
		srcSize = src_fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
		srcSize_uv = src_fmt.fmt.pix_mp.plane_fmt[1].sizeimage;
	}

	return ret;

}

int s_selection()
{
	int ret = 0;
	if (selection.type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		ret = ioctl(fd, VIDIOC_S_SELECTION, &selection);
		if (ret < 0)
			pexit("error setting selection\n");
	}

	return 0;
}

int dst_req_bufs()
{
	int ret = 0;
	struct v4l2_requestbuffers	reqbuf;

	bzero(&reqbuf, sizeof(reqbuf));
	reqbuf.count	= dst_numbuf;
	reqbuf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbuf.memory	= V4L2_MEMORY_MMAP;

	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		pexit("Cant request buffers\n");
	} else {
		dst_numbuf = reqbuf.count;
	}

	return 0;
}

int src_req_bufs()
{
	int ret = 0;
	struct v4l2_requestbuffers	reqbuf;
	
	bzero(&reqbuf, sizeof(reqbuf));
	reqbuf.count	= src_numbuf;
	reqbuf.type	= V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbuf.memory	= V4L2_MEMORY_MMAP;

	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		pexit("Cant request buffers\n");
	} else {
		src_numbuf = reqbuf.count;
	}

	return 0;

}

int src_query_buf(int index, struct v4l2_buffer &buffer)
{
	int ret = 0;
	
	struct v4l2_plane		buf_planes[2];
	
	memset(&buffer, 0, sizeof(buffer));
	buffer.type	= V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	buffer.memory	= V4L2_MEMORY_MMAP;
	buffer.index	= index;
	buffer.m.planes	= buf_planes;
	buffer.length	= src_num_planes;

	ret = ioctl(fd, VIDIOC_QUERYBUF, &buffer);
	if (ret < 0)
		pexit("Cant query buffers\n");

	return 0;
}

int dst_query_buf(int index, struct v4l2_buffer &buffer)
{
	int ret = 0;
	struct v4l2_plane		buf_planes[2];

	memset(&buffer, 0, sizeof(buffer));
	buffer.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	buffer.memory	= V4L2_MEMORY_MMAP;
	buffer.index	= index;
	buffer.m.planes	= buf_planes;
	buffer.length	= dst_num_planes;

	ret = ioctl(fd, VIDIOC_QUERYBUF, &buffer);
	if (ret < 0)
		pexit("Cant query buffers\n");

	return 0;
}

// capture
int allocDstBuffers(void *base[][])
{

	struct v4l2_requestbuffers	reqbuf;
	struct v4l2_buffer		buffer;
	struct v4l2_plane		buf_planes[2];
	int i = 0;
	int j = 0;
	int ret = -1;

	dst_s_fmt();

	s_selection();

	dst_req_bufs();

	for (i = 0; i < dst_numbuf; i++) {
		dst_query_buf(i, buffer);

		for (j = 0; j < src_num_planes; j++) {
			base[i][j]	= mmap(NULL, buffer.m.planes[j].length, PROT_READ | PROT_WRITE, MAP_SHARED,	fd, buffer.m.planes[j].m.mem_offset);
			if (MAP_FAILED == base[i][j]) {
				goto error;
			}
		}
	}

	return 0;

error:
	while (i >= 0) {
		i--;
		while (j >= 0) {
			j--
			munmap(base[i][j], srcSize);
			base[i][j] = NULL;
		}
	}

	return 1;
}

// output
int allocSrcBuffers(void *base[][])
{
	struct v4l2_buffer		buffer;
	struct v4l2_plane		buf_planes[2];
	int i = 0;
	int j = 0;
	int ret = -1;

	src_s_fmt();
	s_selection();
	src_req_bufs();

	for (i = 0; i < src_numbuf; i++) {
		src_query_buf(i, buffer);

		for (j = 0; j < src_num_planes; j++) {
			base[i][j] = mmap(NULL, buffer.m.planes[j].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.planes[j].m.mem_offset);
			if (MAP_FAILED == base[i][j]) {
				goto error;
			}
		}
	}

	return 0;

error:
	while (i >= 0) {
		i--;
		while (j >= 0) {
			j--
			munmap(base[i][j], srcSize);
			base[i][j] = NULL;
		}
	}

	return 1;
}

void releaseBuffers(void	*base[],int	numbuf, int	bufsize)
{
	while (numbuf > 0) {
		numbuf--;
		munmap(base[numbuf], bufsize);
	}
}

int queueAllBuffers(int	type, int numbuf)
{
	struct v4l2_buffer	buffer;
	struct v4l2_plane	buf_planes[2];
	int			i = 0;
	int			ret = -1;
	int			lastqueued = -1;

	for (i = 0; i < numbuf; i++) {
		memset(&buffer, 0, sizeof(buffer));
		buffer.type = type;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;
		buffer.m.planes	= buf_planes;
		buffer.length	= 2;
		gettimeofday(&buffer.timestamp, NULL);

		ret = ioctl(fd, VIDIOC_QBUF, &buffer);
		if (-1 == ret)
			break;
		else
			lastqueued = i;
	}

	return lastqueued;
}


/*
 * description: put a buffer to the buffer queue,
 * 					you can look as : ask the kernel alloc a buffer and put it to the buffer queue
 */
int queue(int type, int index, int field, int num_panels)
{
	int ret = -1;
	struct v4l2_buffer	buffer;
	struct v4l2_plane	buf_planes[2];

	for (int i = 0; i < num_panels; i++) {

		buf_planes[0].length = src_fmt.fmt.pix_mp.plane_fmt[i].sizeimage;
		buf_planes[0].bytesused = src_fmt.fmt.pix_mp.plane_fmt[i].sizeimage;
		buf_planes[0].data_offset = 0;
	}

	memset(&buffer, 0, sizeof(buffer));
	buffer.type	    = type;
	buffer.memory	= V4L2_MEMORY_MMAP;
	buffer.index	= index;
	buffer.m.planes	= buf_planes;
	buffer.field    = field;
	buffer.length	= 2;

	// 投放一个空的视频缓冲区到视频缓冲区输入队列中
	ret = ioctl(fd, VIDIOC_QBUF, &buffer);
	if (-1 == ret)
		pexit("Failed to queue\n");
	return ret;
}

int dequeue(int	type, struct v4l2_buffer *buf, struct v4l2_plane *buf_planes)
{
	int ret = -1;

	memset(buf, 0, sizeof(*buf));
	buf->type	= type;
	buf->memory	= V4L2_MEMORY_MMAP;
	buf->m.planes	= buf_planes;
	buf->length	= 2;
	ret = ioctl(fd, VIDIOC_DQBUF, buf);
	return ret;
}

void streamON(int type)
{
	int	ret = -1;
	ret = ioctl(fd, VIDIOC_STREAMON, &type);
	if (-1 == ret)
		pexit("Cant Stream on\n");
}

void streamOFF(int type)
{
	int	ret = -1;
	ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
	if (-1 == ret)
		pexit("Cant Stream on\n");
}

void do_read(int fd, void *addr, int size)
{
	int nbytes = size, ret = 0, val;
	do {
		nbytes = size - ret;
		addr = (char *)addr + ret;
		if (nbytes == 0)
			break;
		ret = read(fd, addr, nbytes);
	} while (ret > 0);
}

void do_write(int fd, void *addr, int size)
{
	int nbytes = size, ret = 0, val;
	do {
		nbytes = size - ret;
		addr = (char *)addr + ret;
		if (nbytes == 0)
			break;
		ret = write(fd, addr, nbytes);
	} while (ret > 0);
}

static void usage(void)
{
	char localusage[] =
	"Usage: vpetest [-d <Device file|number>] -i <Input> -j <WxH> -k <Pixel Format>\n"
	"-o <Output> -p <WxH> -q <Pixel Format>\n"
	"[-c <top,left,width,height>] [-l [0-2] [-t [1-4]]\n"
	"Convert input video file into desired output format\n"
	"\t[-d <Device file>] : /dev/video0 (default)\n"
	"\t-i <Input>         : Input file name\n"
	"\t-j <WxH>           : Input frame size\n"
	"\t-k <Pixel Format>  : Input frame format\n"
	"\t-o <Output>        : Output file name\n"
	"\t-p <WxH>           : Output frame size\n"
	"\t-q <Pixel Format>  : Output frame format\n"
	"\t[-c <top,left,width,height>]  : Crop target\n"
	"\t[-l [0-2]]         : 0=no deinterlacing (default), 1=alternate field, 2=seq_tb\n"
	"\t[-t <1-4>]         : Number of buffers\n"
	"\t[-v]               : Verbose/Debug\n";

	printf("%s\n", localusage);
}

int parse_parameter(int argc, char *argv[],
	char *devname,
	char *srcfile, int &srcWidth, int &srcHeight, char *srcFmt,
	char *dstfile, int &dstWidth, int &dstHeight, char *dstFmt,
	struct	v4l2_selection &selection, int &interlace, int &translen)
{
	int c;
	int index;
	char shortoptions[] = "d:i:j:k:o:p:q:c:l:t:v";

	for (;;) {
		char *endptr;

		c = getopt_long(argc, argv, shortoptions, NULL, &index);
		if (-1 == c)
			break;
		switch (c) {
		case 0:
			break;
		case 'd':
		case 'D':
			if (isdigit(optarg[0]) && strlen(optarg) <= 3) {
				sprintf(devname, "/dev/video%s", optarg);
			} else if (!strncmp(optarg, "/dev/video", 10)) {
				strcpy(devname, optarg);
			} else {
				printf("ERROR: Device name not recognized: %s\n\n", optarg);
				usage();
				exit(1);
			}
			printf("device_name: %s\n", devname);
			break;
		case 'i':
		case 'I':
			strcpy(srcfile, optarg);
			printf("srcfile: %s\n", srcfile);
			break;
		case 'j':
		case 'J':
			srcWidth = strtol(optarg, &endptr, 10);
			if (*endptr != 'x' || endptr == optarg) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			srcHeight = strtol(endptr + 1, &endptr, 10);
			if (*endptr != 0) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			/* default crop values at first */
			if (selection.r.height == 0) {
				selection.r.top = selection.r.left = 0;
				selection.r.width = srcWidth;
				selection.r.height = srcHeight;
			}
			break;
		case 'k':
		case 'K':
			strcpy(srcFmt, optarg);
			break;
		case 'o':
		case 'O':
			strcpy(dstfile, optarg);
			printf("dstfile: %s\n", dstfile);
			break;
		case 'p':
		case 'P':
			dstWidth = strtol(optarg, &endptr, 10);
			if (*endptr != 'x' || endptr == optarg) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			dstHeight = strtol(endptr + 1, &endptr, 10);
			if (*endptr != 0) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			break;
		case 'q':
		case 'Q':
			strcpy(dstFmt, optarg);
			break;
		case 'c':
		case 'C':
			selection.r.top = strtol(optarg, &endptr, 10);
			if (*endptr != ',' || endptr == optarg) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			selection.r.left = strtol(endptr + 1, &endptr, 10);
			if (*endptr != ',' || endptr == optarg) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			selection.r.width = strtol(endptr + 1, &endptr, 10);
			if (*endptr != ',' || endptr == optarg) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			selection.r.height = strtol(endptr + 1, &endptr, 10);
			if (*endptr != 0) {
				printf("Invalid size '%s'\n", optarg);
				return 1;
			}
			break;
		case 'l':
		case 'L':
			interlace = atoi(optarg);
			break;
		case 't':
		case 'T':
			translen = atoi(optarg);
			break;
		case 'v':
		case 'V':
			debug = 1;
			break;
		default:
			usage();
			exit(1);
		}
	}

}

int s_ctrl(int	translen)
{
	struct	v4l2_control ctrl;

	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id = V4L2_CID_TRANS_NUM_BUFS;
	ctrl.value = translen;
	return ioctl(fd, VIDIOC_S_CTRL, &ctrl);
}

int main(int argc, char *argv[])
{
	
	int ret = 0;
	int i;

	char devname[30];

	char srcfile[256];

	char srcFmt[30];

	char dstfile[256];

	char dstFmt[30];

	void *srcBuffers[6][2];
	void *dstBuffers[6][2];

	int	num_frames = 20;

	int	translen = 3;
	int	frame_no = 0;


	struct	timeval now;
	int	latency;
	int field;
	off_t file_size;

	/* let's setup default values before parsing arguments */
	strcpy(devname, "/dev/video0");
	srcfile[0] = '\0';
	dstfile[0] = '\0';

	selection.r.top = selection.r.left = 0;
	selection.r.width = 0;
	selection.r.height = 0;
	selection.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	selection.target = V4L2_SEL_TGT_CROP_ACTIVE;

	interlace = 0;
	translen = 3;

	// parse parameter
	parse_parameter(argc, argv,
		devname,
		srcfile, srcWidth, srcHeight, srcFmt,
		dstfile, dstWidth, dstHeight, dstFmt,
		selection, interlace, translen);

	// get fmt
	/** Open input file in read only mode */
	fin = open(srcfile, O_RDONLY);
	describeFormat(srcFmt, srcWidth, srcHeight, &srcSize,
		       &srcFourcc, &src_num_planes, &src_colorspace);

	/** Open output file in write mode Create the file if not present */
	fout = open(dstfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	describeFormat(dstFmt, dstWidth, dstHeight, &dstSize,
		       &dstFourcc, &dst_num_planes, &dst_colorspace);

	/** Calculate number of frames **/
	file_size = lseek(fin, 0L, SEEK_END);
	printf("srcsize=%u, filesize=%u", srcSize, file_size);
	num_frames = file_size / srcSize;
	lseek(fin, 0L, SEEK_SET);


	/* SEQ_TB mode is 2 field per buffers so total number of frames will double */
	if (interlace == 2)
		num_frames *= 2;

	printf("Number of frames = %d\n", num_frames);

	if (fin  < 0 || srcHeight < 0 || srcWidth < 0 || srcFourcc < 0 ||
	    fout < 0 || dstHeight < 0 || dstWidth < 0 || dstFourcc < 0) {
		/** TODO:Handle errors precisely		*/
		exit(1);
	}
	/*************************************
		Files are ready Now
	*************************************/

	fd = open(devname, O_RDWR);
	if (fd < 0)
		pexit("Cannot open device\n");

	ret = s_ctrl(translen);
	if (ret < 0)
		pexit("Can't set translen control\n");

	ret = allocSrcBuffers(srcBuffers);

	ret = allocDstBuffers(dstBuffers);
	if (ret < 0)
		pexit("Cant Allocate buffurs for CAPTURE device\n");

	/**	Queue All empty buffers	(Available to capture in)	*/
	ret = queueAllBuffers(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, dst_numbuf);
	if (ret < 0)
		pexit("Error queueing buffers for CAPTURE device\n");

	/*************************************
		Driver is ready Now
	*************************************/


	// 把所有的数据都放入到src buf中
	/**	Read  into the OUTPUT  buffers from fin file	*/

	switch (interlace) {
	case 2:
		field = V4L2_FIELD_SEQ_TB;
		break;
	case 1:
		field = V4L2_FIELD_TOP;
		break;
	default:
		field = V4L2_FIELD_ANY;
		break;
	}

	// 先把输出队列（src）放满，让vpe解析
	for (i = 0; i < src_numbuf && i < num_frames; i++) {
		for (int j = 0; j < src_num_planes; j++) {
			do_read(fin, srcBuffers[i][j], src_fmt.fmt.pix_mp.plane_fmt[j].sizeimage);
		}
		
		queue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, i, field, src_num_planes);

		if (field == V4L2_FIELD_TOP)
			field = V4L2_FIELD_BOTTOM;
		else if (field == V4L2_FIELD_BOTTOM)
			field = V4L2_FIELD_TOP;
	}

	/*************************************
		Data is ready Now
	*************************************/

	streamON(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	streamON(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

	// 
	while (num_frames) {
		struct v4l2_buffer buf;
		struct v4l2_plane buf_planes[2];
		int last = num_frames == 1 ? 1 : 0;
		int iter = interlace == 2 ? 2 : 1;

		/* DEI: Do not Dequeue Source buffers immediately
		 * De*interlacer keeps last two buffers in use */
		if ((interlace && frame_no <= 2)) {
		} else {
			/* Wait for and dequeue one buffer from OUTPUT
			 * to write data for next interlaced field */
			 // 查询一个可以用的buf,拿来装数据，装好数据后又放回去
			dequeue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, &buf, buf_planes);

			// 如果当前是最后一张，那文件里面实际上已经没有数据可以读了？  
			// 这完全不合逻辑啊
			
			if (!last) {
				for (int i = 0; i < src_num_planes; i++) {
					do_read(fin, srcBuffers[buf.index][i], src_fmt.fmt.pix_mp.plane_fmt[i].sizeimage);
				}
				
				queue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, buf.index, field, src_num_planes);
				if (field == V4L2_FIELD_TOP)
					field = V4L2_FIELD_BOTTOM;
				else if (field == V4L2_FIELD_BOTTOM)
					field = V4L2_FIELD_TOP;
			}
		}

		// 对于奇偶行的数据来说，一次要读两个
		while (iter--) {
			/* Dequeue progressive frame from CAPTURE stream
			 * write to the file and queue one empty buffer */
			dequeue(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, &buf, buf_planes);

			for (int i = 0; i < dst_num_planes; i++) {
				do_write(fout, dstBuffers[buf.index][i], dst_fmt.fmt.pix_mp.plane_fmt[i].sizeimage);
			}

			if (!last)
				queue(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, buf.index, V4L2_FIELD_NONE, dst_num_planes);

			num_frames--;
			frame_no++;
		}
	}

	/* Driver cleanup */
	streamOFF(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	streamOFF(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

	releaseBuffers(srcBuffers, src_numbuf, srcSize);
	releaseBuffers(dstBuffers, dst_numbuf, dstSize);

	if (src_num_planes == 2)
		releaseBuffers(srcBuffers_uv, src_numbuf, srcSize_uv);
	if (dst_num_planes == 2)
		releaseBuffers(dstBuffers_uv, dst_numbuf, dstSize_uv);

	close(fin);
	close(fout);
	close(fd);

	return 0;
}

