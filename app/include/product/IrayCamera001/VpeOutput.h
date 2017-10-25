#ifndef _VPE_OUTPUT_H_
#define _VPE_OUTPUT_H_

#include <common/base-def.h>
#include <VpeCommon.h>
#include <VpeDev.h>

class VpeOutput {

public:
	VpeOutput(VpeDev *vpeDev);
	~VpeOutput();

	int init(int width, int height, int pixel_format);
	int init(int width, int height, int pixel_format, int field);
	int init(int width, int height, int pixel_format, struct v4l2_rect &rect);
	int init(int width, int height, int pixel_format, int field, struct v4l2_rect &rect);

	int initAndSetFmt(int width, int height, int pix_fmt, int field);
	int initAndSetSelection(struct v4l2_rect & rect);
	int requestAndSaveBuffer();
	int queryAndMmapBufs();
	
	int query_bufs();
	int req_buf();
	int mmap_buffers();
	void munmap_buffers();
	int mmap_buffer(struct v4l2_buffer &buf, struct v4l2_buffer_user &buf_user);
	void munmap_buffer(struct v4l2_buffer_user & buf_user);
	int initBuffer(struct v4l2_buffer & buf);
	void freeBuffer(struct v4l2_buffer & buf);
	int initBufferManager(struct v4l2_requestbuffers &req_buf);
	void freeBufferManager();
	int initV4l2Buffer(struct v4l2_buffer &buf, struct v4l2_format &fmt);
	int initBufferUser(struct v4l2_buffer_user & buf_usr, int num_planes);
	void freeBufferUser(struct v4l2_buffer_user & buf_usr);
	int initBufferManagerUser(struct v4l2_requestbuffers & req_buf);
	void freeBufferManagerUser();

	int ouput(void *data, u32 len);
	u32 getAFreeBuffer();
	int queueAllBuffers();
	int queue(int index);

private:
	VpeDev *m_vpeDev;

	u32 m_output_index;
	struct v4l2_buffer_user *m_output_buf;

	struct v4l2_format m_fmt;
	struct v4l2_selection m_selection;

	struct v4l2_requestbuffers m_req_buf;
	struct v4l2_buf_mag m_buf_mag;
	struct v4l2_buf_mag_usr m_buf_mag_usr;

	struct v4l2_buffer m_buf_for_dq;
};


#endif
