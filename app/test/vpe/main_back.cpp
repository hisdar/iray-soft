#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <common/Parameter.h>

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

typedef struct vpe_data_format {
	char *name;
	int width;
	int height;
	int size;
	int size_y;
	int size_uv;
	int fourcc;
	int num_planes;
	double bpp;   // bytes per pixel
	enum v4l2_colorspace clrspc;
} iray_fmt;

struct vpe_data_format fmts_list[] = {
	{
		.name       = "rgb24",
		.fourcc     = V4L2_PIX_FMT_RGB24,
		.bpp        = 3,
		.num_planes = 1,
		.clrspc     = V4L2_COLORSPACE_SRGB,
	},
	{
		.name = "bgr24",
		.fourcc = V4L2_PIX_FMT_BGR24,
		.bpp = 3,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SRGB,
	},
	{
		.name = "argb32",
		.fourcc = V4L2_PIX_FMT_RGB32,
		.bpp = 4,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SRGB,

	},
	{
		.name = "abgr32",
		.fourcc = V4L2_PIX_FMT_BGR32,
		.bpp = 4,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SRGB,

	},
	{
		.name = "rgb565",
		.fourcc = V4L2_PIX_FMT_RGB565,
		.bpp = 2,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SRGB,

	},
	{
		.name = "yuv444",
		.fourcc = V4L2_PIX_FMT_YUV444,
		.bpp = 3,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "yvyu",
		.fourcc = V4L2_PIX_FMT_YVYU,
		.bpp = 2,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "yuyv",
		.fourcc = V4L2_PIX_FMT_YUYV,
		.bpp = 2,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "uyvy",
		.fourcc = V4L2_PIX_FMT_UYVY,
		.bpp = 2,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "vyuy",
		.fourcc = V4L2_PIX_FMT_VYUY,
		.bpp = 2,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nv16",
		.fourcc = V4L2_PIX_FMT_NV16,
		.bpp = 2,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nv61",
		.fourcc = V4L2_PIX_FMT_NV61,
		.bpp = 2,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nv12s",
		.fourcc = V4L2_PIX_FMT_NV12,
		.bpp = 1.5,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nv21s",
		.fourcc = V4L2_PIX_FMT_NV21,
		.bpp = 1.5,
		.num_planes = 1,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nv12",
		.fourcc = V4L2_PIX_FMT_NV12,
		.bpp = 1.5,
		.num_planes = 2,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nv21",
		.fourcc = V4L2_PIX_FMT_NV21,
		.bpp = 1.5,
		.num_planes = 2,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nm12",
		.fourcc = V4L2_PIX_FMT_NV12M,
		.bpp = 1.5,
		.num_planes = 2,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
	{
		.name = "nm21",
		.fourcc = V4L2_PIX_FMT_NV21M,
		.bpp = 1.5,
		.num_planes = 2,
		.clrspc = V4L2_COLORSPACE_SMPTE170M,

	},
};
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

int describeFormat(char *format, int width, int height, iray_fmt *fmt)
{
	int i = 0;
	fmt->width = width;
	fmt->height = height;

	struct vpe_data_format *fmt_buf = NULL;
	for (i = 0; i < ARRAY_SIZE(fmts_list), i++) {
		if (strcmp(fmts_list[i].name, format) == 0) {
			fmt_buf = &fmts_list[i];
			break;
		}
	}

	if (fmt_buf == NULL) {
		printf("fmt not found, format=%s\n", format);
		return 1;
	}

	fmt = fmt_buf;

	return 0;
}

int allocBuffers(
	int	type,
	int	width,
	int	height,
	int	num_planes,
	enum v4l2_colorspace clrspc,
	int	*sizeimage_y,
	int	*sizeimage_uv,
	int	fourcc,
	void	*base[],
	void	*base_uv[],
	int	*numbuf,
	int	interlace,
	struct	v4l2_selection s)
{
	struct v4l2_format		fmt;
	struct v4l2_requestbuffers	reqbuf;
	struct v4l2_buffer		buffer;
	struct v4l2_plane		buf_planes[2];
	int				i = 0;
	int				ret = -1;

	bzero(&fmt, sizeof(fmt));
	fmt.type		= type;
	fmt.fmt.pix_mp.width	= width;
	fmt.fmt.pix_mp.height	= height;
	fmt.fmt.pix_mp.pixelformat = fourcc;
	fmt.fmt.pix_mp.colorspace = clrspc;
	fmt.fmt.pix_mp.num_planes = num_planes;

	if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE && interlace == 1)
		fmt.fmt.pix_mp.field = V4L2_FIELD_ALTERNATE;
	else if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE && interlace == 2)
		fmt.fmt.pix_mp.field = V4L2_FIELD_SEQ_TB;
	else
		fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;

	ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		pexit("Cant set color format\n");
	} else {
		*sizeimage_y = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
		*sizeimage_uv = fmt.fmt.pix_mp.plane_fmt[1].sizeimage;
	}

	if (s.type == V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		ret = ioctl(fd, VIDIOC_S_SELECTION, &s);
		if (ret < 0)
			pexit("error setting selection\n");
	}

	dprintf("Buffer Type:%s, requested %dx%d got %dx%d\n",
		V4L2_TYPE_IS_OUTPUT(type)?"output":"capture",
		width, height, fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height);

	bzero(&reqbuf, sizeof(reqbuf));
	reqbuf.count	= *numbuf;
	reqbuf.type	= type;
	reqbuf.memory	= V4L2_MEMORY_MMAP;

	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		pexit("Cant request buffers\n");
	} else {
		*numbuf = reqbuf.count;
	}

	for (i = 0; i < *numbuf; i++) {
		memset(&buffer, 0, sizeof(buffer));
		buffer.type	= type;
		buffer.memory	= V4L2_MEMORY_MMAP;
		buffer.index	= i;
		buffer.m.planes	= buf_planes;
		buffer.length	= num_planes;

		ret = ioctl(fd, VIDIOC_QUERYBUF, &buffer);
		if (ret < 0)
			pexit("Cant query buffers\n");

		printf("query buf, plane 0 = %d\n",
		       buffer.m.planes[0].length);
		if (num_planes == 2)
			printf("           plane 1 = %d\n",
			       buffer.m.planes[1].length);

		base[i]	= mmap(NULL, buffer.m.planes[0].length,
				PROT_READ | PROT_WRITE, MAP_SHARED,
				fd, buffer.m.planes[0].m.mem_offset);

		if (MAP_FAILED == base[i]) {
			while (i >= 0) {
				/* Unmap all previous buffers */
				i--;
				munmap(base[i], *sizeimage_y);
				base[i] = NULL;
			}
			pexit("Cant mmap buffers Y");
			return 0;
		}

		if (num_planes == 1)
			continue;

		base_uv[i] = mmap(NULL, buffer.m.planes[1].length,
				  PROT_READ | PROT_WRITE, MAP_SHARED,
				  fd, buffer.m.planes[1].m.mem_offset);

		if (MAP_FAILED == base_uv[i]) {
			while (i >= 0) {
				/* Unmap all previous buffers */
				i--;
				munmap(base_uv[i], *sizeimage_uv);
				base[i] = NULL;
			}
			pexit("Cant mmap buffers UV");
			return 0;
		}
	}

	return 1;
}

void releaseBuffers(
	void	*base[],
	int	numbuf,
	int	bufsize)
{
	while (numbuf > 0) {
		numbuf--;
		munmap(base[numbuf], bufsize);
	}
}

int queueAllBuffers(
	int	type,
	int	numbuf
	)
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

int queue(
	int	type,
	int	index,
	int	field,
	int	size_y,
	int size_uv)
{
	struct v4l2_buffer	buffer;
	struct v4l2_plane	buf_planes[2];
	int			ret = -1;

	dprintf("DBG queue(%d, %d, %d, %d, %d)\n",
		type, index, field, size_y, size_uv);
	buf_planes[0].length = buf_planes[0].bytesused = size_y;
	buf_planes[1].length = buf_planes[1].bytesused = size_uv;
	buf_planes[0].data_offset = buf_planes[1].data_offset = 0;

	memset(&buffer, 0, sizeof(buffer));
	buffer.type	= type;
	buffer.memory	= V4L2_MEMORY_MMAP;
	buffer.index	= index;
	buffer.m.planes	= buf_planes;
	buffer.field    = field;
	buffer.length	= 2;

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

void do_read(char *str, int fd, void *addr, int size)
{
	int nbytes = size, ret = 0, val;
	do {
		nbytes = size - ret;
		addr = addr + ret;
		if (nbytes == 0)
			break;
		ret = read(fd, addr, nbytes);
	} while (ret > 0);

	if (ret < 0) {
		val = errno;
		printf("Reading failed %s: %d %s\n", str, ret, strerror(val));
		exit(1);
	} else {
		dprintf("Total bytes read %s = %d\n", str, size);
	}
}

void do_write(char *str, int fd, void *addr, int size)
{
	int nbytes = size, ret = 0, val;
	do {
		nbytes = size - ret;
		addr = addr + ret;
		if (nbytes == 0)
			break;
		ret = write(fd, addr, nbytes);
	} while (ret > 0);
	if (ret < 0) {
		val = errno;
		printf("Writing failed %s: %d %s\n", str, ret, strerror(val));
		exit(1);
	} else {
		dprintf("Total bytes written %s = %d\n", str, size);
	}
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

int main(int argc, char *argv[])
{
	int c, ret = 0;
	int index;
	int i;
	char devname[30];
	char	srcfile[256];
	char	dstfile[256];
	int	srcHeight  = 0, dstHeight = 0;
	int	srcWidth   = 0, dstWidth  = 0;
	int srcSize_uv = 0, dstSize_uv = 0;
	char    srcFmt[30], dstFmt[30];

	void	*srcBuffers[6];
	void	*dstBuffers[6];
	void	*srcBuffers_uv[6];
	void	*dstBuffers_uv[6];
	int	src_numbuf = 6;
	int	dst_numbuf = 6;
	int	num_frames = 20;
	int	interlace = 0;
	int	translen = 3;
	int	frame_no = 0;
	struct	v4l2_control ctrl;
	struct	v4l2_selection selection;
	struct	timeval now;
	int	latency;
	int field;
	off_t file_size;

	iray_fmt src_fmt;
	iray_fmt dst_fmt;

	Parameter p;
	p.parse(argc, argv);

	cout << p.toString() << endl;

	/* let's setup default values before parsing arguments */
	strcpy(devname, "/dev/video0");
	srcfile[0] = '\0';
	dstfile[0] = '\0';
	parse_input(argc, argv);

	selection.r.top = selection.r.left = 0;
	selection.r.width = 0;
	selection.r.height = 0;
	selection.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	selection.target = V4L2_SEL_TGT_CROP_ACTIVE;
	interlace = 0;
	translen = 3;


	/** Open input file in read only mode */
	fin = open(srcfile, O_RDONLY);
	describeFormat(srcFmt, srcWidth, srcHeight, &src_fmt);

	/** Open output file in write mode Create the file if not present */
	fout = open(dstfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	describeFormat(dstFmt, dstWidth, dstHeight, &dst_fmt);

	/** Calculate number of frames **/
	file_size = lseek(fin, 0L, SEEK_END);
	//file_size = 737280;
	//printk("srcsize=%u, filesize=%u", srcSize, file_size);
	num_frames = file_size / src_fmt.size;
	lseek(fin, 0L, SEEK_SET);


	printf("Input  @ %d = %d x %d , %d\nOutput @ %d = %d x %d , %d\n",
	       fin,  srcWidth, srcHeight, src_fmt.fourcc,
	       fout, dstWidth, dstHeight, dst_fmt.fourcc);

	printf("Crop/Selection top: %d left: %d width %d height: %d\n",
	       selection.r.top, selection.r.left,
	       selection.r.width, selection.r.height);

	printf("Interlace: %d\n", interlace);

	/* SEQ_TB mode is 2 field per buffers so total number of frames will double */
	if (interlace == 2)
		num_frames *= 2;

	printf("Number of frames = %d\n", num_frames);

	if (fin  < 0 || srcHeight < 0 || srcWidth < 0 || src_fmt.fourcc < 0 ||
	    fout < 0 || dstHeight < 0 || dstWidth < 0 || dst_fmt.fourcc < 0) {
		/** TODO:Handle errors precisely		*/
		exit(1);
	}
	/*************************************
		Files are ready Now
	*************************************/

	fd = open(devname, O_RDWR);
	if (fd < 0)
		pexit("Cannot open device\n");

	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id = V4L2_CID_TRANS_NUM_BUFS;
	ctrl.value = translen;
	ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		pexit("Can't set translen control\n");

	ret = allocBuffers(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, srcWidth,
		srcHeight, src_fmt.num_planes, src_fmt.clrspc, &src_fmt.size, &srcSize_uv,
		src_fmt.fourcc, srcBuffers, srcBuffers_uv, &src_numbuf, interlace,
		selection);
	if (ret < 0)
		pexit("Cant Allocate buffurs for OUTPUT device\n");

	ret = allocBuffers(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, dstWidth,
		dstHeight, dst_fmt.num_planes, dst_fmt.clrspc, &dst_fmt.size, &dstSize_uv,
		dst_fmt.fourcc, dstBuffers, dstBuffers_uv, &dst_numbuf, interlace,
		selection);
	if (ret < 0)
		pexit("Cant Allocate buffurs for CAPTURE device\n");

	/**	Queue All empty buffers	(Available to capture in)	*/
	ret = queueAllBuffers(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, dst_numbuf);
	if (ret < 0)
		pexit("Error queueing buffers for CAPTURE device\n");

	printf("Input  Buffers = %d each of size %d\nOutput Buffers = %d each of size %d\n",
	       src_numbuf, src_fmt.size, dst_numbuf, dst_fmt.size);

	/*************************************
		Driver is ready Now
	*************************************/

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
	for (i = 0; i < src_numbuf && i < num_frames; i++) {
		do_read("Y plane", fin, srcBuffers[i], src_fmt.size);
		if (src_fmt.num_planes == 2)
			do_read("UV plane", fin, srcBuffers_uv[i], srcSize_uv);

		queue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, i, field, src_fmt.size,
		      srcSize_uv);
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

	if (!debug) {
		printf("frames left %04d", num_frames);
		fflush(stdout);
	}
	while (num_frames) {
		struct v4l2_buffer buf;
		struct v4l2_plane buf_planes[2];
		int last = num_frames == 1 ? 1 : 0;
		int iter = interlace == 2 ? 2 : 1;

		/* DEI: Do not Dequeue Source buffers immediately
		 * De*interlacer keeps last two buffers in use */
		if (!(interlace && frame_no <= 2)) {
			/* Wait for and dequeue one buffer from OUTPUT
			 * to write data for next interlaced field */
			dequeue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, &buf,
				buf_planes);
			dprintf("dequeued source buffer with index %d\n",
				buf.index);

			if (!last) {
				do_read("Y plane", fin, srcBuffers[buf.index],
					src_fmt.size);
				if (src_fmt.num_planes == 2)
					do_read("UV plane", fin,
						srcBuffers_uv[buf.index],
						srcSize_uv);

				queue(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
				      buf.index, field, src_fmt.size, srcSize_uv);
				if (field == V4L2_FIELD_TOP)
					field = V4L2_FIELD_BOTTOM;
				else if (field == V4L2_FIELD_BOTTOM)
					field = V4L2_FIELD_TOP;
			}
		}

		while (iter--) {
			/* Dequeue progressive frame from CAPTURE stream
			 * write to the file and queue one empty buffer */
			dequeue(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, &buf,
				buf_planes);
			dprintf("dequeued dest buffer with index %d\n",
				buf.index);

			gettimeofday(&now, NULL);
			latency = now.tv_usec - buf.timestamp.tv_usec;
			if (latency < 0)
				latency += 1000000;
			latency += (now.tv_sec - buf.timestamp.tv_sec) *
				   1000000;
			dprintf("Latency = %7d us\n", latency);

			do_write("Y plane", fout, dstBuffers[buf.index],
				 dst_fmt.size);
			if (dst_fmt.num_planes == 2)
				do_write("UV plane", fout,
					 dstBuffers_uv[buf.index], dstSize_uv);

			if (!last)
				queue(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
				      buf.index, V4L2_FIELD_NONE,
				      dst_fmt.size, dstSize_uv);

			num_frames--;
			frame_no++;

			if (debug) {
				printf("frames left %04d\n", num_frames);
				fflush(stdout);
			} else {
				printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bframes left %04d", num_frames);
				fflush(stdout);
			}
		}
	}

	printf("\n");

	/* Driver cleanup */
	streamOFF(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	streamOFF(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

	releaseBuffers(srcBuffers, src_numbuf, src_fmt.size);
	releaseBuffers(dstBuffers, dst_numbuf, dst_fmt.size);

	if (src_fmt.num_planes == 2)
		releaseBuffers(srcBuffers_uv, src_numbuf, srcSize_uv);
	if (dst_fmt.num_planes == 2)
		releaseBuffers(dstBuffers_uv, dst_numbuf, dstSize_uv);

	close(fin);
	close(fout);
	close(fd);

	return 0;
}

