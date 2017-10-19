
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <memory.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "base-def.h"
#include "IrayCamera.h"

IrayCamera::IrayCamera()
{
	m_frame_receiver = NULL;
	m_is_print_frame_info = 0;
//	m_field_type = V4L2_FIELD_NONE;
	m_field_type = V4L2_FIELD_ALTERNATE;

	memset(&m_frame_buf, 0x00, sizeof(v4l2_user_buffer));
}

IrayCamera::~IrayCamera()
{

}

void IrayCamera::setFrameReceiver(IrayCameraRcv *fr)
{
	m_frame_receiver = fr;
}

/**
 * @parameter: count - capture count, if count is 0, no stop
 */
int IrayCamera::startCapture(int count, u32 isPrintInfo)
{
	int ret = 0;

	m_is_print_frame_info = isPrintInfo;
	
	ret = open();
	if (ret) {
		iray_err("open fail, ret=%d\n", ret);
		return ret;
	}

	ret = getFormatDefault();
	if (ret) {
		iray_err("get format fail, ret=%d\n", ret);
		return ret;
	}

	ret = setFormatDefault();
	if (ret) {
		iray_err("set format fail, ret=%d\n", ret);
		return ret;
	}

	ret = getFormatDefault();
	if (ret) {
		iray_err("getFormatDefault fain, ret=%d\n", ret);
		return ret;
	}

	ret = reqBufs();
	if (ret) {
		iray_err("reqBufs fail, ret=%d\n", ret);
		return ret;
	}

	ret = mmaps();
	if (ret) {
		iray_dbg("mmaps fail, ret=%d\n", ret);
		return ret;
	}

	ret = qBufs();

	ret = streamOn();
	if (ret) {
		return ret;
	}
	
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
	printf("| index	 =%u\n", m_v4l2_buf.index);
	printf("| type	  =%u\n", m_v4l2_buf.type);
	printf("| bytesused =%u\n", m_v4l2_buf.bytesused);
	printf("| flags	 =%u\n", m_v4l2_buf.flags);
	printf("| field	 =%u\n", m_v4l2_buf.field);
	printf("| sequence  =%u\n", m_v4l2_buf.sequence);
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

	iray_dbg("fmt width       : [%u]\n", m_v4l2_fmt.fmt.pix.width);
	iray_dbg("fmt height      : [%u]\n", m_v4l2_fmt.fmt.pix.height);
	iray_dbg("fmt pixelformat : [%u]\n", m_v4l2_fmt.fmt.pix.pixelformat);
	iray_dbg("fmt field       : [%u]\n", m_v4l2_fmt.fmt.pix.field);
	iray_dbg("fmt bytesperline: [%u]\n", m_v4l2_fmt.fmt.pix.bytesperline);
	iray_dbg("fmt sizeimage   : [%u]\n", m_v4l2_fmt.fmt.pix.sizeimage);
	iray_dbg("fmt colorspace  : [%u]\n", m_v4l2_fmt.fmt.pix.colorspace);
	iray_dbg("fmt priv        : [%u]\n", m_v4l2_fmt.fmt.pix.priv);
	iray_dbg("fmt flags       : [%u]\n", m_v4l2_fmt.fmt.pix.flags);
	iray_dbg("fmt ycbcr_enc   : [%u]\n", m_v4l2_fmt.fmt.pix.ycbcr_enc);
	iray_dbg("fmt quantization: [%u]\n", m_v4l2_fmt.fmt.pix.quantization);
	iray_dbg("fmt xfer_func   : [%u]\n", m_v4l2_fmt.fmt.pix.xfer_func);

	return SUCCESS;
}

int IrayCamera::setFormatDefault()
{
	m_v4l2_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	m_v4l2_fmt.fmt.pix.field = m_field_type;
	m_v4l2_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	iray_dbg("---------------------------------\n");
	iray_dbg("set pix.field=%d\n", m_v4l2_fmt.fmt.pix.field);
	iray_dbg("set pix.pixelformat=%d\n", m_v4l2_fmt.fmt.pix.pixelformat);

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
	
	m_v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_v4l2_buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(m_cam_fd, VIDIOC_DQBUF, &m_v4l2_buf);
	if (ret < 0) {
		iray_err("ioctl VIDIOC_DQBUF error, ret=%d", ret);
		return -ENOIOCT;
	}

	// this place, should check the following code
	// save_file(m_user_buffer[m_v4l2_buf.index].addr, m_user_buffer[m_v4l2_buf.index].length, i);
	if (m_frame_receiver != NULL) {
		notifyFrameData(&m_user_buffer[m_v4l2_buf.index]);
	}

	if (m_is_print_frame_info) {
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

int IrayCamera::notifyFrameData(v4l2_user_buffer *srcData)
{
	int ret = 0;

	m_frame_data.setFormat(&m_v4l2_fmt);

	if (m_v4l2_fmt.fmt.pix.field == V4L2_FIELD_ALTERNATE) {
		ret = initFrameBuffer(srcData);
		if (ret) {
			iray_err("initFrameBuffer, ret=%d\n", ret);
			return ret;
		}

		int image_width = m_v4l2_fmt.fmt.pix.width * 2;
		//int image_height = m_v4l2_fmt.fmt.pix.height;

		char *tag_addr = NULL;
		char *src_addr = srcData->addr;
		
		if (m_v4l2_buf.sequence % 2 == 0) {
			tag_addr = m_frame_buf.addr;
			for	(int i = 0; i < 288; i++) {
				memcpy(tag_addr, src_addr, image_width);
				tag_addr += image_width * 2;
				src_addr += image_width;
			}
		} else {
			tag_addr = m_frame_buf.addr + image_width;
			for	(int i = 0; i < 288; i++) {
				memcpy(tag_addr, src_addr, image_width);
				tag_addr += image_width * 2;
				src_addr += image_width;
			}

			if (m_frame_buf.addr == NULL) {
				iray_err("buf addr is null\n");
			}
			m_frame_data.setAddr(m_frame_buf.addr);
			m_frame_data.setLength(m_frame_buf.length);
		}
	} else {
		m_frame_data.setAddr(srcData->addr);
		m_frame_data.setLength(srcData->length);
	}

	m_frame_receiver->receiveFrame(&m_frame_data);

	return 0;
}

int IrayCamera::initFrameBuffer(v4l2_user_buffer *srcData)
{
	if (m_frame_buf.length != srcData->length) {
		CHECK_FREE(m_frame_buf.addr);

		m_frame_buf.length = srcData->length;
		m_frame_buf.addr = (char *)malloc(srcData->length);
		if (m_frame_buf.addr == NULL) {
			iray_err("alloc mem fail:[%d]%s, size=%d\n", errno, strerror(errno), srcData->length);
			return -ENOMEM;
		}
	}

	return SUCCESS;
}

void IrayCamera::setFieldType(u32 fieldType)
{
	m_field_type = fieldType;
}

