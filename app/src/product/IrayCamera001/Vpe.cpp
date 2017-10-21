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

int Vpe::init(int srcWidth, int srcHeight, int srcPixFmt,
			int dstWidth, int dstHeight, int dstPixFmt)
{
	return 0;
}

int Vpe::init(int srcWidth, int srcHeight, int srcPixFmt,
			int dstWidth, int dstHeight, int dstPixFmt,
				struct v4l2_rect &rect)
{
	return 0;
}

int Vpe::init(int srcWidth, int srcHeight, int srcPixFmt,
			int dstWidth, int dstHeight, int dstPixFmt,
			int field)
{

	return 0;
}

int Vpe::init(int srcWidth, int srcHeight, int srcPixFmt,
			int dstWidth, int dstHeight, int dstPixFmt,
			int field, struct v4l2_rect &rect)
{
	int ret = 0;
	struct v4l2_pix_format_mplane &src_pix = m_src_fmt.fmt.pix;
	struct v4l2_pix_format_mplane &dst_pix = m_dst_fmt.fmt.pix;

	// check field
	if (field != V4L2_FIELD_ALTERNATE
		&& field != V4L2_FIELD_SEQ_TB) {
		iray_err("check field value fail, field=%d\n", field);
		return -ENOPARA;	
	}

	m_src_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	src_pix.field  = field;
	ret = initPixFmt(srcWidth, srcHeight, srcPixFmt, src_pix);
	if (ret) {
		iray_err("init src pix fmt fail, ret=%d\n", ret);
		return ret;
	}
	
	m_dst_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	dst_pix.field  = V4L2_FIELD_ANY;
	ret = initPixFmt(dstWidth, dstHeight, dstPixFmt, dst_pix);
	if (ret) {
		iray_err("init dst pix fmt fail, ret=%d\n", ret);
		return ret;
	}

	memcpy(&m_selection.r, &rect, sizeof(struct v4l2_rect));
	m_selection.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	m_selection.target = V4L2_SEL_TGT_CROP_ACTIVE;
	
	return SUCCESS;
}

int Vpe::initPixFmt(int width, int height, int pixFmt, struct v4l2_pix_format_mplane &fmt)
{
	int ret = 0;
	struct vpe_color_type color_type = {0};
	
	ret = findColorTypeByPixFmt(pixFmt, &color_type);
	if (ret) {
		return ret;
	}

	fmt.width = width;
	fmt.height = height;
	fmt.colorspace = color_type.clrspc;
	fmt.pixelformat = color_type.pixelformat;
	fmt.num_planes = color_type.num_planes;
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

int Vpe::s_ctrl(int translen)
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

/*
 * description : 
 */
int Vpe::allocBuffers()
{
	// set format
	
	// set selection

	// request queue buffers

	// query buffers

	// mmap buffers
}

int Vpe::findColorTypeByName(char *name, struct vpe_color_type *fmt)
{
	u32 i = 0;

	struct vpe_color_type *fmt_buf = NULL;
	for (i = 0; i < ARRAY_SIZE(color_type_list); i++) {
		if (strcmp(color_type_list[i].name, name) == 0) {
			memcpy(fmt, &color_type_list[i], sizeof(struct vpe_color_type));
			return 0;
		}
	}

	iray_err("fmt not found, color type=%s\n", name);
	return -ENODATA;
}

int Vpe::findColorTypeByPixFmt(int pixelformat, struct vpe_color_type *fmt)
{
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(color_type_list); i++) {
		if (color_type_list[i].pixelformat == pixelformat) {
			memcpy(fmt, &color_type_list[i], sizeof(struct vpe_color_type));
			return 0;
		}
	}

	iray_err("fmt not found, color type=%s\n", name);
	return -ENODATA;
}


