#ifndef _VPE_H_
#define _VPE_H_

#include <common/IrayImage.h>

#define VPE_DEVICE_PATH "/dev/video0"

#define NUM_OF_SRC_BUFFERS		(6)
#define NUM_OF_DST_BUFFERS		(6)

struct vpe_color_type {
	const char *name;
	int pixelformat;
	int num_planes;
	double bpp;   // bytes per pixel
	enum v4l2_colorspace clrspc;
};

struct vpe_color_type color_type_list[] = {
	// name    pixelformat         num_planes bpp clrspc
	{"rgb24",  V4L2_PIX_FMT_RGB24, 1,         3,  V4L2_COLORSPACE_SRGB},
	{"bgr24",  V4L2_PIX_FMT_BGR24, 1,         3,  V4L2_COLORSPACE_SRGB},
	{"argb32", V4L2_PIX_FMT_RGB32, 1,         4,  V4L2_COLORSPACE_SRGB},
	{"abgr32", V4L2_PIX_FMT_BGR32, 1,         4,  V4L2_COLORSPACE_SRGB},
	{"rgb565", V4L2_PIX_FMT_RGB565,1,         2,  V4L2_COLORSPACE_SRGB},
	{"yuv444", V4L2_PIX_FMT_YUV444,1,         3,  V4L2_COLORSPACE_SMPTE170M},
	{"yvyu",   V4L2_PIX_FMT_YVYU,  1,         2,  V4L2_COLORSPACE_SMPTE170M},
	{"yuyv",   V4L2_PIX_FMT_YUYV,  1,         2,  V4L2_COLORSPACE_SMPTE170M},
	{"uyvy",   V4L2_PIX_FMT_UYVY,  1,         2,  V4L2_COLORSPACE_SMPTE170M},
	{"vyuy",   V4L2_PIX_FMT_VYUY,  1,         2,  V4L2_COLORSPACE_SMPTE170M},
	{"nv16",   V4L2_PIX_FMT_NV16,  1,         2,  V4L2_COLORSPACE_SMPTE170M},
	{"nv61",   V4L2_PIX_FMT_NV61,  1,         2,  V4L2_COLORSPACE_SMPTE170M},
	{"nv12s",  V4L2_PIX_FMT_NV12,  1,         1.5,V4L2_COLORSPACE_SMPTE170M},
	{"nv21s",  V4L2_PIX_FMT_NV21,  1,         1.5,V4L2_COLORSPACE_SMPTE170M},
	{"nv12",   V4L2_PIX_FMT_NV12,  2,         1.5,V4L2_COLORSPACE_SMPTE170M},
	{"nv21",   V4L2_PIX_FMT_NV21,  2,         1.5,V4L2_COLORSPACE_SMPTE170M},
	{"nm12",   V4L2_PIX_FMT_NV12M, 2,         1.5,V4L2_COLORSPACE_SMPTE170M},
	{"nm21",   V4L2_PIX_FMT_NV21M, 2,         1.5,V4L2_COLORSPACE_SMPTE170M}
};

struct v4l2_plane_user {
	void *addr;
	u32  length;
	u8   is_mapped;
};

struct v4l2_buffer_user {
	u8 is_queued;
	u32 num_planes;
	struct v4l2_plane_user *planes;
};

class Vpe {
public:
	Vpe();
	~Vpe();

	int init(int srcWidth, int srcHeight, int srcPixFmt, int dstWidth, int dstHeight, int dstPixFmt);
	int init(int srcWidth, int srcHeight, int srcPixFmt, int dstWidth, int dstHeight, int dstPixFmt, int field);
	int init(int srcWidth, int srcHeight, int srcPixFmt, int dstWidth, int dstHeight, int dstPixFmt, struct v4l2_rect & rect);
	int init(int srcWidth, int srcHeight, int srcPixFmt, int dstWidth, int dstHeight, int dstPixFmt, int field, struct v4l2_rect & rect);

	int initV4l2Buffer(struct v4l2_buffer & buf, struct v4l2_format & fmt);
	int q_buf(int index, struct v4l2_buffer & buf);

	int queueDstBuffers();
	int initUserBuffer(struct v4l2_buffer_user & buf, int num_planes);
	void releaseUserBuffer(struct v4l2_buffer_user &buf);
	int initUserBuffers(struct v4l2_buffer_user * buf, int buf_len, int num_planes);
	int initPixFmt(int width, int height, int pixFmt, struct v4l2_pix_format_mplane & fmt);
	void munmapBuffer(struct v4l2_buffer_user & buf);
	int mmapBuffer(int type, int index, struct v4l2_buffer_user & buf);
	int allocBuffers(struct v4l2_buffer_user * buf, int num_buffers, struct v4l2_format & fmt);

	int s_fmt(struct v4l2_format & fmt);
	int s_selection();
	int req_bufs(int type, int num_buffers);
	int stream_on(int type);
	int query_planes(int type, int index, struct v4l2_plane * planes, int num_planes);
	int findColorTypeByName(char * name, struct vpe_color_type * fmt);
	int findColorTypeByPixFmt(int pixelformat, struct vpe_color_type * fmt);
	int getDstLength();
	struct v4l2_buffer_user *getAFreebuffer();
	int put(void * data [ ], int data_len);
	int get();
	int process();
	
	int formatSwap(IrayImage &src, IrayImage &dst);
	int open();
	int s_ctrl();

private:
	int m_dev;

	u32 m_src_num_buffers;
	u32 m_dst_num_buffers;

	struct v4l2_buffer_user m_src_buf_user[NUM_OF_SRC_BUFFERS];
	struct v4l2_buffer_user m_dst_buf_user[NUM_OF_SRC_BUFFERS];
	
	struct v4l2_format m_src_fmt;
	struct v4l2_format m_dst_fmt;
	struct v4l2_selection m_selection;

	// those two buffers are used for quick queue and dequeue
	// do not use for query !!!
	struct v4l2_buffer m_src_buffer;
	struct v4l2_buffer m_dst_buffer;
};

#endif
