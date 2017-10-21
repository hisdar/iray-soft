#ifndef _VPE_H_
#define _VPE_H_

#include <common/IrayImage.h>

#define VPE_DEVICE_PATH "/dev/video0"

typedef struct vpe_color_type {
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

class Vpe {
public:
	Vpe();
	~Vpe();
	int formatSwap(IrayImage &src, IrayImage &dst);
	int open();

	int s_ctrl(int translen);

private:
	int m_dev;

	struct v4l2_buffer m_buffer;
	
	struct v4l2_format m_src_fmt;
	struct v4l2_format m_dst_fmt;
	struct v4l2_selection m_selection;
	
	struct v4l2_requestbuffers	m_reqbuf;
};

#endif
