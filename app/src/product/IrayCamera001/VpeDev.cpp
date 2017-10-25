#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>
#include <sys/ioctl.h>

#include <common/base-def.h>
#include <VpeCommon.h>
#include <VpeDev.h>

#define V4L2_CID_TRANS_NUM_BUFS (V4L2_CID_USER_TI_VPE_BASE + 0)

struct pix_fmt_info g_pix_fmt_infos[] = {
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
	{"nm21",   V4L2_PIX_FMT_NV21M, 2,         1.5,V4L2_COLORSPACE_SMPTE170M},
};

VpeDev::VpeDev()
{
	
}

VpeDev::~VpeDev()
{
	close();
}

int VpeDev::init(const char *path)
{
	int ret = 0;
	struct v4l2_control ctrl = {0};

	ret = open(path);
	if (ret) {
		iray_err("open device fail, path=%s, ret=%d\n", path, ret);
		return ret;
	}

	ctrl.id    = V4L2_CID_TRANS_NUM_BUFS;
	ctrl.value = TRANS_LEN;
	ret = s_ctrl(ctrl);
	if (ret) {
		iray_err("s_ctrl fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int VpeDev::open(const char *path)
{
	m_dev = ::open(path, O_RDWR);
	if (m_dev <= 0) {
		return -ENODEV;
	}

	return SUCCESS;
}

void VpeDev::close()
{
	::close(m_dev);
}

int VpeDev::q_buf(struct v4l2_buffer &buf)
{
	int ret = ioctl(m_dev, VIDIOC_QBUF, &buf);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::dq_buf(struct v4l2_buffer &buf)
{
	int ret = ioctl(m_dev, VIDIOC_DQBUF, &buf);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::s_ctrl(struct v4l2_control &ctrl)
{
	int ret = ioctl(m_dev, VIDIOC_S_CTRL, &ctrl);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::s_fmt(struct v4l2_format &fmt)
{
	int ret = ioctl(m_dev, VIDIOC_S_FMT, &fmt);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::s_selection(struct v4l2_selection selection)
{
	int ret = ioctl(m_dev, VIDIOC_S_SELECTION, &selection);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::req_bufs(v4l2_requestbuffers &req_buf)
{
	int ret = ioctl(m_dev, VIDIOC_REQBUFS, &req_buf);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::stream_on(u32 &type)
{
	int ret = ioctl(m_dev, VIDIOC_STREAMON, &type);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::stream_off(u32 &type)
{
	int ret = ioctl(m_dev, VIDIOC_STREAMOFF, &type);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

int VpeDev::query_buf(struct v4l2_buffer &buf)
{
	int ret = ioctl(m_dev, VIDIOC_QUERYBUF, &buf);
	if (ret) {
		return errno;
	}

	return SUCCESS;
}

void *VpeDev::mmap(size_t length, off_t offset)
{
	return ::mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, m_dev, offset);
}

void VpeDev::munmap(void *addr, size_t length)
{
	::munmap(addr, length);
}

int VpeDev::getPixFmtInfo(int pix_fmt, struct pix_fmt_info &fmt_info)
{
	for (u32 i = 0; i < ARRAY_SIZE(g_pix_fmt_infos); i++) {
		if (g_pix_fmt_infos[i].pix_fmt == pix_fmt) {
			memcpy(&fmt_info, &g_pix_fmt_infos[i], sizeof(struct pix_fmt_info));
			return 0;
		}
	}

	iray_err("fmt not found, pixelformat=%d\n", pix_fmt);
	return -ENODATA;
}


