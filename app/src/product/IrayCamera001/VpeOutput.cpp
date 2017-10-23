#include <VpeOutput.h>

VpeOutput::VpeOutput(int dev, Vpe *vpe)
{
	m_dev = dev;
	m_vpe = vpe;
}

VpeOutput::~VpeOutput()
{
	
}

int VpeOutput::init(int width, int height, int pixel_format)
{
	return SUCCESS;

}

int VpeOutput::init(int width, int height, int pixel_format, int field)
{
	return SUCCESS;

}

int VpeOutput::init(int width, int height, int pixel_format, struct v4l2_rect &rect)
{
	return SUCCESS;

}

int VpeOutput::init(int width, int height, int pixel_format, int field, struct v4l2_rect &rect)
{
	int ret = 0;

	struct v4l2_pix_format_mplane &pix = m_fmt.fmt.pix_mp;

	// check field
	if (field != V4L2_FIELD_ALTERNATE
		&& field != V4L2_FIELD_SEQ_TB
		&& field != V4L2_FIELD_ANY) {
		iray_err("check field value fail, field=%d\n", field);
		return -ENOPARA;
	}

	m_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	pix.field  = field;
	pix.width = width;
	pix.height = height;
	ret = m_vpe->initPixFmt(pixel_format, pix);
	if (ret) {
		iray_err("init src pix fmt fail, ret=%d\n", ret);
		return ret;
	}

	ret = m_vpe->s_fmt(m_fmt);

	// init selection
	memcpy(&m_selection.r, &rect, sizeof(struct v4l2_rect));
	m_selection.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	m_selection.target = V4L2_SEL_TGT_CROP_ACTIVE;
	ret = s_selection(m_selection);

	// init buffer 
	//ret = initV4l2Buffer(m_buffer, m_fmt);
	m_num_planes = pix.num_planes;
	m_planes = (struct v4l2_plane *)malloc(sizeof(struct v4l2_plane) * m_num_planes);
	if (m_planes == NULL) {
		return -ENOMEM;
	}

	// request buffers
	
	m_req_buf.type   = m_fmt.type;
	m_req_buf.memory = V4L2_MEMORY_MMAP;
	m_req_buf.count  = NUM_OF_SRC_BUFFERS;
	ret = m_vpe->req_bufs(m_req_buf);

	// init a memory to store the requested buffers infor
	ret = query_buf();

	// mmap buffers

	return SUCCESS;
}

int VpeOutput::query_buf()
{
	int ret = 0;
	struct v4l2_buffer buf = {0};

	memset(&buf, 0, sizeof(buf));
	buf.type     = m_fmt.type;
	buf.memory	 = V4L2_MEMORY_MMAP;
	buf.m.planes = m_planes;
	buf.length	 = m_num_planes;

	for (int i = 0; i < m_req_buf.count; i++) {
		buf.index = i;
		ret = query_buf(buf);
		if (ret) {
			iray_err("src query buf fail, ret=%d\n", ret);
		}

		// save addr, length, index, planes
	}

	return ret;
}

int VpeOutput::initV4l2Buffer(struct v4l2_buffer &buf, struct v4l2_format &fmt)
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


