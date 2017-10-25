#ifndef _VPE_CAPTURE_H_
#define _VPE_CAPTURE_H_

#include <common/base-def.h>
#include <VpeCommon.h>
#include <VpeDev.h>

class VpeCapture {
public:
	VpeCapture(VpeDev *vpeDev);
	~VpeCapture();

	int init(int width, int height, int pixel_format);

	int initAndSetFmt(int width, int height, int pix_fmt);
	int requestAndSaveBuffer();

	int initBuffer(struct v4l2_buffer & buf);
	void freeBuffer(struct v4l2_buffer & buf);
	int initBufferManager(struct v4l2_requestbuffers & req_buf);
	void freeBufferManager();

	int initBufferUser(struct v4l2_buffer_user & buf_usr, int num_planes);
	void freeBufferUser(struct v4l2_buffer_user & buf_usr);
	int initBufferManagerUser(struct v4l2_requestbuffers & req_buf);
	void freeBufferManagerUser();

	int req_buf();
	int queryAndMmapBufs();
	int query_bufs();
	int mmap_buffer(struct v4l2_buffer & buf, struct v4l2_buffer_user & buf_user);
	int mmap_buffers();

	void munmap_buffer(struct v4l2_buffer_user & buf_user);
	void munmap_buffers();

	int capture(void *data, u32 len);
	void printV4l2Buffer(struct v4l2_buffer &buf);
	int queueAllBuffers();
	int queue(int index);

private:

	VpeDev *m_vpeDev;
	struct v4l2_format m_fmt;
	struct v4l2_requestbuffers m_req_buf;
	struct v4l2_buf_mag m_buf_mag;
	struct v4l2_buf_mag_usr m_buf_mag_usr;

	struct v4l2_buffer m_buf_for_dq;
};
#endif
