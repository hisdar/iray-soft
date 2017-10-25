#ifndef _VPE_DEV_H_
#define _VPE_DEV_H_

#include <sys/mman.h>
#include <linux/videodev2.h>

#define TRANS_LEN 3

class VpeDev {
public:
	VpeDev();
	~VpeDev();

	int open(const char * path);
	void close();

	int init(const char * path);
	
	int q_buf(struct v4l2_buffer & buf);
	int dq_buf(struct v4l2_buffer & buf);
	int s_ctrl(struct v4l2_control &ctrl);
	int s_fmt(struct v4l2_format & fmt);
	int s_selection(struct v4l2_selection selection);
	int req_bufs(v4l2_requestbuffers & req_buf);
	int query_buf(struct v4l2_buffer & buf);
	int stream_on(u32 & type);
	int stream_off(u32 & type);
	void *mmap(size_t length, off_t offset);
	void munmap(void *addr, size_t length);

	int getPixFmtInfo(int pix_fmt, struct pix_fmt_info &fmt_info);

private:
	int m_dev;
};
#endif
