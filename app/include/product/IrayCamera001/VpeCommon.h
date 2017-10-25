#ifndef _VPE_COMMON_H_
#define _VPE_COMMON_H_

#include <linux/videodev2.h>

#define NUM_OF_SRC_BUFFERS		(2)
#define NUM_OF_DST_BUFFERS		(4)

struct pix_fmt_info {
	const char *name;
	int pix_fmt;
	int num_planes;
	double bpp;   // bytes per pixel
	enum v4l2_colorspace clrspc;
};

struct v4l2_plane_user {
	void *addr;
	u32  length;
	u8   is_mapped;
};

struct v4l2_buffer_user {
	u8 is_queued;
	u32 index;
	u32 num_planes;
	struct v4l2_plane_user *planes;
};

struct v4l2_buf_mag_usr {
	u32 size;
	struct v4l2_buffer_user *buf_users;
};

struct v4l2_buf_mag {
	u32 size;
	struct v4l2_buffer *bufs;
};


#endif
