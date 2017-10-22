#include <fcntl.h>
#include <memory.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <common/base-def.h>
#include <linux/v4l2-controls.h>

#include <Vpe.h>

#define V4L2_CID_TRANS_NUM_BUFS (V4L2_CID_USER_TI_VPE_BASE + 0)
#define TRANS_LEN               (3) // <1-4>

Vpe::Vpe()
{

	memset(m_src_buf_user, 0x00, sizeof(struct v4l2_buffer_user) * NUM_OF_SRC_BUFFERS);
	memset(m_dst_buf_user, 0x00, sizeof(struct v4l2_buffer_user) * NUM_OF_DST_BUFFERS);
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
	struct v4l2_pix_format_mplane &src_pix = m_src_fmt.fmt.pix_mp;
	struct v4l2_pix_format_mplane &dst_pix = m_dst_fmt.fmt.pix_mp;

	// check field
	if (field != V4L2_FIELD_ALTERNATE
		&& field != V4L2_FIELD_SEQ_TB) {
		iray_err("check field value fail, field=%d\n", field);
		return -ENOPARA;	
	}

	m_src_num_buffers = NUM_OF_SRC_BUFFERS;
	m_dst_num_buffers = NUM_OF_DST_BUFFERS;

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

	// init selection
	memcpy(&m_selection.r, &rect, sizeof(struct v4l2_rect));
	m_selection.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	m_selection.target = V4L2_SEL_TGT_CROP_ACTIVE;

	// init user buffers
	ret = initUserBuffers(m_src_buf_user, NUM_OF_SRC_BUFFERS, m_src_fmt.fmt.pix_mp.num_planes);
	ret = initUserBuffers(m_dst_buf_user, NUM_OF_DST_BUFFERS, m_dst_fmt.fmt.pix_mp.num_planes);

	// init v4l2_buffer
	ret = initV4l2Buffer(m_src_buffer, m_src_fmt);
	ret = initV4l2Buffer(m_dst_buffer, m_dst_fmt);

	// open device, because we are going to set it
	ret = open();
	if (ret) {
		return ret;
	}

	// set
	ret = s_ctrl();
	if (!ret) {
		iray_err("s_ctrl fail, ret=%d\n", ret);
		return ret;
	}

	// alloc buffers
	ret = allocBuffers(m_src_buf_user, NUM_OF_SRC_BUFFERS, m_src_fmt);

	ret = allocBuffers(m_dst_buf_user, NUM_OF_DST_BUFFERS, m_dst_fmt);

	// queue dst buffers:
	// the dst buffers is used to receive the process result,
	// so we should put the buffers to the queue befor start stream
	ret = queueDstBuffers();

	// now, we can stream on
	ret = stream_on(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	ret = stream_on(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

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
	buf.length   = num_planes;
	buf.m.planes = planes;

	return SUCCESS;
}

int Vpe::q_buf(int index, struct v4l2_buffer &buf)
{
	int ret = SUCCESS;

	buf.index    = index;
	buf.memory   = V4L2_MEMORY_MMAP;

	gettimeofday(&buf.timestamp, NULL);
	
	ret = ioctl(m_dev, VIDIOC_QBUF, &buf);
	if (ret) {
		iray_err("queue buffers fail, ret=%d, index=%d\n", ret, index);
	}

	return ret;	
}


// TODO: if not set the planes, what the value of planes after dqbuf ?
/*int dq_buf(struct v4l2_buffer *buf)
{
	int ret = -1;

	memset(buf, 0, sizeof(struct v4l2_buffer));
	buf->type	  = type;
	buf->memory	  = V4L2_MEMORY_MMAP;
	buf->m.planes = buf_planes;
	buf->length	  = 2;
	ret = ioctl(fd, VIDIOC_DQBUF, buf);
	return ret;
}*/

int Vpe::queueDstBuffers()
{
	int ret = 0;
	for (int i = 0; i < NUM_OF_DST_BUFFERS; i++) {
		ret = q_buf(i, m_dst_buffer);
		if (ret) {
			iray_err("queue buffer fail, index=%d\n", i);
			return ret;
		}
	}

	return SUCCESS;
}


int Vpe::initUserBuffer(struct v4l2_buffer_user &buf, int num_planes)
{
	buf.num_planes = num_planes;
	buf.planes     = (struct v4l2_plane_user *)malloc(sizeof(struct v4l2_plane_user) *  num_planes);
	if (NULL == buf.planes) {
		return -ENOMEM;
	}

	memset(buf.planes, 0x00, sizeof(struct v4l2_plane_user) * buf.num_planes);
	return SUCCESS;
}

void Vpe::releaseUserBuffer(struct v4l2_buffer_user &buf)
{
	CHECK_FREE(buf.planes);
	buf.num_planes = 0;
}

int Vpe::initUserBuffers(struct v4l2_buffer_user *buf, int buf_len, int num_planes)
{
	int i = 0;
	int ret = SUCCESS;

	// clear the buffer
	memset(buf, 0x00, sizeof(struct v4l2_buffer_user) * buf_len);

	// alloc mem for every buffer planes
	for (i = 0; i < buf_len; i++) {
		ret = initUserBuffer(buf[i], num_planes);
		if (ret) {
			iray_err("init User buffer fail, ret=%d, index=%d\n", ret, i);
			break;
		}
	}

	// error handle
	if (i != buf_len) {
		for (; i >= 0; i--) {
			releaseUserBuffer(buf[i]);
		}
	}
	
	return ret;
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

	return SUCCESS;
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
	struct v4l2_control ctrl = {0};
	
	ctrl.id    = V4L2_CID_TRANS_NUM_BUFS;
	ctrl.value = TRANS_LEN;

	ret = ioctl(m_dev, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0) {
		iray_err("set ctrl fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

void Vpe::munmapBuffer(struct v4l2_buffer_user &buf)
{
	for (u32 i = 0; i < buf.num_planes; i++) {
		if (buf.planes[i].is_mapped) {
			munmap(buf.planes[i].addr, buf.planes[i].length);
		}
	}
}

int Vpe::mmapBuffer(int type, int index, struct v4l2_buffer_user &buf)
{
	u32 i = 0;
	int ret = SUCCESS;
	void *mmap_addr = NULL;
	struct v4l2_plane *planes = NULL;

	planes = (struct v4l2_plane *)malloc(sizeof(struct v4l2_plane) * buf.num_planes);
	if (NULL == planes) {
		iray_err("alloc mem for plane fail\n");
		return -ENOMEM;
	}

	// query planes infor, because we will mmap all the planes to user space
	ret = query_planes(type, index, planes, buf.num_planes);
	if (ret) {
		iray_err("query planes fail, ret=%d\n", ret);

		CHECK_FREE(planes);
		return ret;
	}

	for (i = 0; i < buf.num_planes; i++) {
		mmap_addr = mmap(NULL, planes[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, m_dev, planes[i].m.mem_offset);
		if (MAP_FAILED == mmap_addr) {
			iray_err("mmap fail, i=%d, num_planes=%d\n", i, buf.num_planes);
			break;
		}
		
		buf.planes[i].addr = mmap_addr;
		buf.planes[i].length = planes[i].length;
		buf.planes[i].is_mapped = TRUE;
	}

	if (i != buf.num_planes) {
		munmapBuffer(buf);
		ret = -ENOMEM;
	}

	CHECK_FREE(planes);
	return ret;
}

/*
 * description : 
 */
int Vpe::allocBuffers(struct v4l2_buffer_user *buf, int num_buffers, struct v4l2_format &fmt)
{
	int i = 0;
	int ret = SUCCESS;

	ret = s_fmt(fmt);
	if (ret) {
		return ret;
	}

	ret = s_selection();
	if (ret) {
		return ret;
	}

	ret = req_bufs(fmt.type, num_buffers);
	if (ret) {
		return ret;
	}

	for (i = 0; i < num_buffers; i++) {
		ret = mmapBuffer(fmt.type, i, buf[i]);
		if (ret) {
			break;
		}
	}

	// error handle
	if (i != num_buffers) {
		for (; i >= 0; i--) {
			munmapBuffer(buf[i]);
		}	
	}

	return ret;
}

int Vpe::s_fmt(struct v4l2_format &fmt)
{
	int ret = 0;

	ret = ioctl(m_dev, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		iray_err("set src fmt fail, ret=%d\n", ret);
		return ret;
	} 

	return SUCCESS;
}

int Vpe::s_selection()
{
	int ret = 0;

	ret = ioctl(m_dev, VIDIOC_S_SELECTION, &m_selection);
	if (ret < 0) {
		iray_err("set dst selection fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int Vpe::req_bufs(int type, int num_buffers)
{
	int ret = 0;
	struct v4l2_requestbuffers reqbuf = {0};
	
	reqbuf.type	  = type;
	reqbuf.count  = num_buffers;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(m_dev, VIDIOC_REQBUFS, &reqbuf);
	if ((ret < 0) || ((u32)num_buffers != reqbuf.count)) {
		iray_err("request buffers fail, ret=%d, num_buffers=%d, reqbuf.count=%d\n", ret, num_buffers, reqbuf.count);
		return ret;
	}

	return 0;
}

int Vpe::stream_on(int type)
{
	int	ret = SUCCESS;
	ret = ioctl(m_dev, VIDIOC_STREAMON, &type);
	if (ret) {
		iray_err("stream on fail, type=%d\n", type);
	}

	return ret;
}

int Vpe::query_planes(int type, int index, struct v4l2_plane *planes, int num_planes)
{
	int ret = 0;
	struct v4l2_buffer buf = {0};

	memset(&buf, 0, sizeof(buf));
	buf.type     = type;
	buf.memory	 = V4L2_MEMORY_MMAP;
	buf.index	 = index;
	buf.m.planes = planes;
	buf.length	 = num_planes;

	ret = ioctl(m_dev, VIDIOC_QUERYBUF, &buf);
	if (ret < 0) {
		iray_err("src query buf fail, ret=%d\n", ret);
	}

	return ret;
}

int Vpe::findColorTypeByName(char *name, struct vpe_color_type *fmt)
{
	for (u32 i = 0; i < ARRAY_SIZE(color_type_list); i++) {
		if (strcmp(color_type_list[i].name, name) == 0) {
			memcpy(fmt, &color_type_list[i], sizeof(struct vpe_color_type));
			return 0;
		}
	}

	iray_err("fmt not found, name=%s\n", name);
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

	iray_err("fmt not found, pixelformat=%d\n", pixelformat);
	return -ENODATA;
}

int Vpe::getDstLength()
{
	// TODO: 1. check init
	// TODO: 2. find a good way to get dst length
	return m_dst_buf_user[0].planes[0].length;
}

// TODO: this function can be optimized 
struct v4l2_buffer_user *Vpe::getAFreebuffer()
{
	// get a buffer to queue
	for (int i = 0; i < NUM_OF_SRC_BUFFERS; i++) {
		if (!m_src_buf_user[i].is_queued) {
			return &m_src_buf_user[i];
		}
	}

	return NULL;
}

int Vpe::put(void *data[], int data_len)
{
	int ret = 0;
	int num_planes =  m_src_fmt.fmt.pix_mp.num_planes;

	if (data_len != num_planes) {
		iray_err("data len error, data len should be:[%d]\n", num_planes);
		return -ENODATA;
	}

	struct v4l2_buffer_user *buf = getAFreebuffer();
	if (buf == NULL) {
		iray_err("no free buffer to use\n");
		return -ENOMEM;
	}

	// TODO: re-code
	//ret = q_buf(m_src_fmt, index);	
	if (ret) {
		return ret;
	}
	
	return SUCCESS;
}

int Vpe::get()
{
	return 0;
}

int Vpe::process()
{
	return 0;
}

