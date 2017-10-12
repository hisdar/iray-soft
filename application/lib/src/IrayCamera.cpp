
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <memory.h>
#include <fcntl.h>

#include "base-def.h"
#include "IrayCamera.h"

IrayCamera::IrayCamera()
{
	frameReceiver = NULL;
	m_isPrintFrameInfo = 0;
}

IrayCamera::~IrayCamera()
{

}

void IrayCamera::setFrameReceiver(IrayCameraRcv *fr)
{
	frameReceiver = fr;
}

/**
 * @parameter: count - capture count, if count is 0, no stop
 */
int IrayCamera::startCapture(int count, u32 isPrintInfo)
{
	int ret = 0;

	m_isPrintFrameInfo = isPrintInfo;

	
	ret = open();
	if (ret) {
		return ret;
	}
	
	iray_dbg("open camera\n");

	ret = getFormatDefault();
	if (ret) {
		return ret;
	}

	ret = setFormatDefault();
	if (ret) {
		return ret;
	}

	iray_dbg("setFormatDefault success\n");

	ret = getFormatDefault();
	if (ret) {
		return ret;
	}
	iray_dbg("getFormatDefault success\n");


	ret = reqBufs();
	if (ret) {
		return ret;
	}

	iray_dbg("reqBufs success\n");

	ret = mmaps();
	if (ret) {
		return ret;
	}

	iray_dbg("mmaps success\n");

	ret = qBufs();

	ret = streamOn();
	if (ret) {
		return ret;
	}
	
	iray_dbg("streamOn success\n");
	if (count == 0) {
		while (1) {
			dQueBuf();
		}
	} else {
		for (int i = 0; i < count; i++) {
			dQueBuf();
		}
	}
	
	return SUCCESS;
}

int IrayCamera::printFrameInfo()
{
	printf("------------------------------------\n");
	printf("|index     =%u\n", m_v4l2_buf.index);
	printf("|type      =%u\n", m_v4l2_buf.type);
	printf("|bytesused =%u\n", m_v4l2_buf.bytesused);
	printf("|flags     =%u\n", m_v4l2_buf.flags);
	printf("|field     =%u\n", m_v4l2_buf.field);
	printf("------------------------------------\n");

	return 0;
}

int IrayCamera::open()
{
	m_cam_fd = ::open(CAMERA_PATH, O_RDWR);
	if (m_cam_fd == -1) {
		iray_err("open error\n");
		return -ENOIOCT;
	}
	
	return SUCCESS;
}

int IrayCamera::close()
{

	return true;
}

int IrayCamera::setParameter()
{
	return true;
}


int IrayCamera::getFormat(struct v4l2_format *fmt)
{
	int ret = 0;

    fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
    ret = ioctl(m_cam_fd, VIDIOC_G_FMT, fmt);
    if (ret < 0) {
        iray_err("ioctl VIDIOC_G_FMT error, ret=%d\n", ret);
        return -ENOIOCT;
    }

	return SUCCESS;
}

int IrayCamera::getFormatDefault()
{
	int ret = 0;

	memset(&m_v4l2_fmt, 0x00, sizeof(m_v4l2_fmt));
	ret = getFormat(&m_v4l2_fmt);
	if (ret) {
		return ret;
	}

	iray_dbg("frame width: [%u]\n", m_v4l2_fmt.fmt.pix.width);
    iray_dbg("frame height: [%u]\n", m_v4l2_fmt.fmt.pix.height);
	iray_dbg("frame field: [%u]\n", m_v4l2_fmt.fmt.pix.field);
	//iray_dbg("frame field: [%u]\n", fmt.fmt.pix.field);

	return SUCCESS;
}

int IrayCamera::setFormatDefault()
{
	m_v4l2_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	m_v4l2_fmt.fmt.pix.width = IMAGE_WIDTH;
	m_v4l2_fmt.fmt.pix.height = IMAGE_HEIGHT;
	m_v4l2_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;

	return setFormat(&m_v4l2_fmt);
}

int IrayCamera::setFormat(struct v4l2_format *fmt)
{
	int ret = 0;
	
	ret = ioctl(m_cam_fd, VIDIOC_S_FMT, fmt);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_S_FMT error, ret=%d\n", ret);
		return -ENOIOCT;
	}

	return SUCCESS;
}

int IrayCamera::reqBufs()
{
	int ret = 0;
	
	// init buffers
	memset(&m_v4l2_req_bufs, 0x00, sizeof(m_v4l2_req_bufs));
	m_v4l2_req_bufs.count  = DEFAULT_BUFFER_SIZE;
	m_v4l2_req_bufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_v4l2_req_bufs.memory = V4L2_MEMORY_MMAP;
	
	ret = ioctl(m_cam_fd, VIDIOC_REQBUFS, &m_v4l2_req_bufs);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_REQBUFS error, ret=%d\n", ret);
		return -ENOIOCT;
	}

	return SUCCESS;
}

int IrayCamera::queryBuf(int index, struct v4l2_buffer *buf)
{
	int ret = 0;
	
	buf->index = index;
	buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->memory = V4L2_MEMORY_MMAP;

	ret = ioctl(m_cam_fd, VIDIOC_QUERYBUF, buf);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_QUERYBUF error, ret=%d\n", ret);
		return -ENOIOCT;
	}

	return SUCCESS;
}

int IrayCamera::mmap(int index, struct v4l2_buffer *buf)
{
	m_user_buffer[index].index = index;
	m_user_buffer[index].length = buf->length;

	m_user_buffer[index].addr = (char *)::mmap(NULL,
											buf->length,
											PROT_READ | PROT_WRITE,
											MAP_SHARED,
											m_cam_fd,
											buf->m.offset);
	
	if (m_user_buffer[index].addr == MAP_FAILED) {
		iray_err("mmap error\n");
		return -ENOIOCT;
	}
	
	iray_dbg("user_buffer index[%d],len[%d],addr[%p]\n",
		m_user_buffer[index].index, m_user_buffer[index].length, m_user_buffer[index].addr);

	return SUCCESS;
}

int IrayCamera::mmaps()
{
	int ret = 0;
	
	for (u32 i = 0; i < m_v4l2_req_bufs.count; i++) {
		ret = queryBuf(i, &m_v4l2_buf);
		if (ret != SUCCESS) {
			return ret;
		}

		ret = mmap(i, &m_v4l2_buf);
		if (ret != SUCCESS) {
			return ret;
		}
	}

	return SUCCESS;
}

int IrayCamera::qBuf(struct v4l2_buffer *v4l2_buf)
{
	int ret = 0;

	ret = ioctl(m_cam_fd, VIDIOC_QBUF, &m_v4l2_buf);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_QBUF error, ret=%d\n", ret);
		return -ENOIOCT;
	}

	return SUCCESS;
}

int IrayCamera::qBufs()
{
	int ret = 0;

	for (u32 i = 0; i < m_v4l2_req_bufs.count; i ++) {
		m_v4l2_buf.index = i;
		m_v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		m_v4l2_buf.memory = V4L2_MEMORY_MMAP;

		ret = qBuf(&m_v4l2_buf);
		if (ret) {
			return ret;
		}
	}

	return SUCCESS;
}

int IrayCamera::dQueBuf()
{
	int ret = 0;
	IrayCameraData frameData;
		
	m_v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_v4l2_buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(m_cam_fd, VIDIOC_DQBUF, &m_v4l2_buf);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_DQBUF error, ret=%d", ret);
		return -ENOIOCT;
	}

	// this place, should check the following code
	// save_file(m_user_buffer[m_v4l2_buf.index].addr, m_user_buffer[m_v4l2_buf.index].length, i);
	if (frameReceiver != NULL) {
		frameData.setFormat(&m_v4l2_fmt);
		frameData.setAddr(m_user_buffer[m_v4l2_buf.index].addr);
		frameData.setLength(m_user_buffer[m_v4l2_buf.index].length);
		
		frameReceiver->receiveFrame(&frameData);
	}

	if (m_isPrintFrameInfo) {
		printFrameInfo();
	}

	// so we need the following code, because, we should put the buffer back to the queue
	m_v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_v4l2_buf.memory = V4L2_MEMORY_MMAP;
	m_v4l2_buf.m.offset = (unsigned long)m_user_buffer[m_v4l2_buf.index].addr;
	ret = ioctl(m_cam_fd, VIDIOC_QBUF, &m_v4l2_buf);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_DQBUF error, ret=%d\n", ret);
		return -ENOIOCT;
	}
	
	return SUCCESS;
}

int IrayCamera::streamOn()
{
	int ret = 0;

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(m_cam_fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_STREAMOFF error, ret=%d\n", ret);
		return -ENOIOCT;
	}

	return SUCCESS;
}

int IrayCamera::streamOff()
{
	int ret = 0;

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(m_cam_fd, VIDIOC_STREAMOFF, &type);
    if (ret < 0) {
        iray_err("ioctl VIDIOC_STREAMOFF error, ret=%d\n", ret);
        return -ENOIOCT;
    }

	return SUCCESS;
}


