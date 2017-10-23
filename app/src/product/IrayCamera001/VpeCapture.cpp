#include <VpeCapture.h>

VpeCapture::VpeCapture(int dev, Vpe *vpe)
{
	m_dev = dev;
	m_vpe = vpe;
}

VpeCapture::~VpeCapture()
{
	
}

int VpeOutput::init(int width, int height, int pixel_format)
{
	int ret = 0;

	struct v4l2_pix_format_mplane &pix = m_fmt.fmt.pix_mp;


	m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	pix.field  = V4L2_FIELD_ANY;
	pix.width  = width;
	pix.height = height;
	ret = m_vpe->initPixFmt(pixel_format, pix);
	if (ret) {
		iray_err("init src pix fmt fail, ret=%d\n", ret);
		return ret;
	}

	ret = initV4l2Buffer(m_buffer, m_fmt);

	ret = m_vpe->s_fmt(m_fmt);
	
	return SUCCESS;
}

int Vpe::initV4l2Buffer(struct v4l2_buffer &buf, struct v4l2_format &fmt)
{
	int num_planes = 0;
	struct v4l2_plane *planes = NULL;

	num_planes = fmt.fmt.pix_mp.num_planes;
	planes = (struct v4l2_plane *)malloc(sizeof(struct v4l2_plane) * num_planes);
	if (NULL == planes) {
		iray_err("malloc memory for plane fail\n");
		return -ENOMEM;
	}

	memset(&buf, 0x00, sizeof(struct v4l2_buffer));
	buf.type     = fmt.type;
	buf.memory   = V4L2_MEMORY_MMAP;
	buf.field    = fmt.fmt.pix_mp.field;
	buf.length   = num_planes;
	buf.m.planes = planes;

	return SUCCESS;
}


