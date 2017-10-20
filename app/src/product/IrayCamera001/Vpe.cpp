#include <fcntl.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <common/base-def.h>
#include <linux/v4l2-controls.h>

#include <Vpe.h>

#define V4L2_CID_TRANS_NUM_BUFS (V4L2_CID_USER_TI_VPE_BASE + 0)


Vpe::Vpe()
{

}

Vpe::~Vpe()
{

}

int Vpe::formatSwap(IrayImage &src, IrayImage &dst)
{


	return 0;
}

int Vpe::open()
{
	m_dev = ::open(VPE_DEVICE_PATH, O_RDWR);
	if (m_dev <= 0) {
		iray_err("open vpe device fail, path=%s, ret=%d\n", VPE_DEVICE_PATH, m_dev);
		return -ENODEV;
	}

	return SUCCESS;
}

int Vpe::s_ctrl()
{
	int ret = 0;
	struct v4l2_control ctrl;
	
	memset(&ctrl, 0, sizeof(ctrl));

	ctrl.id = V4L2_CID_TRANS_NUM_BUFS;
	ctrl.value = 3;

	ret = ioctl(m_dev, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0) {
		iray_err("set ctrl fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}




