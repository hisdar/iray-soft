#ifndef _VPE_H_
#define _VPE_H_

#include <common/base-def.h>
#include <linux/videodev2.h>
#include <common/IrayImage.h>

#define VPE_DEVICE_PATH "/dev/video0"

#define NUM_OF_SRC_BUFFERS		(2)
#define NUM_OF_DST_BUFFERS		(2)

struct vpe_color_type {
	const char *name;
	int pixelformat;
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

struct v4l2_buffer_manager {
	u32 size;
	struct v4l2_buffer_user *buf_users;
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
	int q_buf(int index, struct v4l2_buffer & buf, struct v4l2_buffer_manager &buf_mag);
	int dq_buf(struct v4l2_buffer_manager &buf_mag, struct v4l2_buffer &buf);

	int getAFreeV4l2Buffer(struct v4l2_buffer_user **buf);

	int queueAllBuffers(struct v4l2_buffer_manager &buf_mag, struct v4l2_buffer &buf);
	int initUserBuffer(struct v4l2_buffer_user & buf, int num_planes);
	void releaseUserBuffer(struct v4l2_buffer_user &buf);
	int initUserBuffers(struct v4l2_buffer_manager * buf_mag, int buf_len, int num_planes);
	int initPixFmt(int pixFmt, struct v4l2_pix_format_mplane & fmt);
	void munmapBuffer(struct v4l2_buffer_user & buf);
	int mmapBuffer(int type, int index, struct v4l2_buffer_user & buf);
	int allocBuffers(struct v4l2_buffer_manager *buf_mag, int num_buffers, struct v4l2_format & fmt);

	int s_fmt(struct v4l2_format & fmt);
	int s_selection();
	int req_bufs(int type, int num_buffers);
	int stream_on(int type);
	int query_planes(int type, int index, struct v4l2_plane * planes, int num_planes);
	int findColorTypeByName(char * name, struct vpe_color_type * fmt);
	int findColorTypeByPixFmt(int pixelformat, struct vpe_color_type * fmt);
	int getDstLength();
	int put(void *data[], u32 data_len);
	int get(char *data[], u32 data_len);
	int process();
	
	int formatSwap(IrayImage &src, IrayImage &dst);
	int open();
	int s_ctrl();

private:
	int m_dev;

	struct v4l2_buffer_manager m_src_buf_mag;
	struct v4l2_buffer_manager m_dst_buf_mag;
	
	struct v4l2_format m_src_fmt;
	struct v4l2_format m_dst_fmt;
	struct v4l2_selection m_selection;

	// those two buffers are used for quick queue and dequeue
	// do not use for query !!!
	struct v4l2_buffer m_src_buffer;
	struct v4l2_buffer m_dst_buffer;
};

#endif
