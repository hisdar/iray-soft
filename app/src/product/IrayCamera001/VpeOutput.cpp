#include <memory.h>
#include <malloc.h>
#include <sys/mman.h>

#include <VpeOutput.h>

VpeOutput::VpeOutput(VpeDev *vpeDev)
{
	m_output_index = 0;
	m_output_buf = NULL;

	m_vpeDev = vpeDev;
}

VpeOutput::~VpeOutput()
{
	freeBufferManagerUser();
	freeBufferManager();
	freeBuffer(m_buf_for_dq);
	munmap_buffers();
}

int VpeOutput::init(int width, int height, int pixel_format)
{
	struct v4l2_rect r;

	r.left = 0;
	r.top  = 0;
	r.width = width;
	r.height = height;
	return init(width, height, pixel_format, V4L2_FIELD_ANY, r);
}

int VpeOutput::init(int width, int height, int pixel_format, int field)
{
	struct v4l2_rect r;

	r.left   = 0;
	r.top    = 0;
	r.width  = width;
	r.height = height;
	return init(width, height, pixel_format, field, r);
}

int VpeOutput::init(int width, int height, int pixel_format, struct v4l2_rect &rect)
{
	return init(width, height, pixel_format, V4L2_FIELD_ANY, rect);
}

int VpeOutput::init(int width, int height, int pixel_format, int field, struct v4l2_rect &rect)
{
	int ret = 0;

	// check field
	if (field != V4L2_FIELD_ALTERNATE
		&& field != V4L2_FIELD_SEQ_TB
		&& field != V4L2_FIELD_ANY) {
		iray_err("check field value fail, field=%d\n", field);
		return -ENOPARA;
	}

	// 1.init and set fmt
	ret = initAndSetFmt(width, height, pixel_format, field);
	if (ret) {
		iray_err("initAndSetFmt fail, ret=%d\n", ret);
		return ret;
	}

	// 2.init and set selection
	ret = initAndSetSelection(rect);
	if (ret) {
		iray_err("initAndSetSelection fail, ret=%d\n", ret);
		return ret;
	}

	// 3. request buffers and save buffer info
	ret = requestAndSaveBuffer();
	if (ret) {
		iray_err("requestAndSaveBuffer fail, ret=%d\n", ret);
		return ret;
	}

	// 4. query buffers and mmap buffers
	ret = queryAndMmapBufs();
	if (ret) {
		iray_err("queryAndMmapBufs fail, ret=%d\n", ret);
		return ret;
	}

	// 5. queue all the buffers
	ret = queueAllBuffers();
	if (ret) {
		iray_err("queueAllBuffers fail, ret=%d\n", ret);
		return ret;
	}

	// 6. stream on
	ret = m_vpeDev->stream_on(m_fmt.type);
	if (ret) {
		iray_err("stream on fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int VpeOutput::queueAllBuffers()
{
	int ret = 0;

	int num_planes = 0;
	struct v4l2_plane *planes_buf = NULL;
	

	for (u32 i = 0; i < m_buf_mag.size; i++) {

		num_planes = m_buf_mag.bufs[i].length;
		planes_buf = m_buf_mag.bufs[i].m.planes;

		memset(&m_buf_mag.bufs[i], 0x00, sizeof(struct v4l2_buffer));
		m_buf_mag.bufs[i].type     = m_fmt.type;
		m_buf_mag.bufs[i].memory   = V4L2_MEMORY_MMAP;
		m_buf_mag.bufs[i].index    = i;
		m_buf_mag.bufs[i].field    = V4L2_FIELD_ANY;
		m_buf_mag.bufs[i].m.planes = planes_buf;
		m_buf_mag.bufs[i].length   = num_planes;
		gettimeofday(&m_buf_mag.bufs[i].timestamp, NULL);

		ret = m_vpeDev->q_buf(m_buf_mag.bufs[i]);
		if (ret) {
			iray_err("stream_on fail, ret=%d, index=%d\n", ret, i);
			return ret;
		}

		// printV4l2Buffer(m_buf_mag.bufs[i]);
		m_buf_mag_usr.buf_users[i].is_queued = TRUE;
	}

	return SUCCESS;
}

int VpeOutput::queue(int index)
{
	int num_planes = 0;
	struct v4l2_buffer *buf = NULL;
	struct v4l2_plane  *planes = NULL;

	buf = &m_buf_mag.bufs[index];
	num_planes = buf->length;
	planes = buf->m.planes;

	/*buf_planes[0].length = buf_planes[0].bytesused = size_y;
	buf_planes[1].length = buf_planes[1].bytesused = size_uv;
	buf_planes[0].data_offset = buf_planes[1].data_offset = 0;
	*/
	memset(buf, 0, sizeof(struct v4l2_buffer));
	buf->type     = m_fmt.type;
	buf->memory   = V4L2_MEMORY_MMAP;
	buf->index    = index;
	buf->field    = V4L2_FIELD_ANY;
	buf->m.planes = planes;
	buf->length   = num_planes;

	return m_vpeDev->q_buf(*buf);
}

int VpeOutput::requestAndSaveBuffer()
{
	int ret = 0;

	ret = req_buf();
	if (ret) {
		iray_err("request buffer fail, ret=%d\n", ret);
		return ret;
	}

	// init buffer for dqueue
	ret = initBuffer(m_buf_for_dq);
	if (ret) {
		iray_err("initBuffer for dq fail, ret=%d\n", ret);
		return ret;
	}

	ret = initBufferManager(m_req_buf);
	if (ret) {
		iray_err("initBufferManager fail, ret=%d\n", ret);
		return ret;
	}

	ret = initBufferManagerUser(m_req_buf);
	if (ret) {
		iray_err("initBufferManagerUser fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int VpeOutput::initAndSetSelection(struct v4l2_rect &rect)
{
	int ret = 0;

	// init selection
	memcpy(&m_selection.r, &rect, sizeof(struct v4l2_rect));
	m_selection.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	m_selection.target = V4L2_SEL_TGT_CROP_ACTIVE;
	ret = m_vpeDev->s_selection(m_selection);
	if (ret) {
		iray_err("set selection fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int VpeOutput::queryAndMmapBufs()
{
	int ret = 0;

	// init a memory to store the requested buffers infor
	ret = query_bufs();
	if (ret) {
		iray_err("query_bufs fail, ret=%d\n", ret);
		return ret;
	}

	// mmap buffers
	ret = mmap_buffers();
	if (ret) {
		iray_err("mmap_buffers fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int VpeOutput::initAndSetFmt(int width, int height, int pix_fmt, int field)
{
	int ret = 0;
	struct v4l2_pix_format_mplane &pix = m_fmt.fmt.pix_mp;

	struct pix_fmt_info pix_fmt_info;
	ret = m_vpeDev->getPixFmtInfo(pix_fmt, pix_fmt_info);
	if (ret) {
		iray_err("getPixFmtInfo fail, pix fmt=%d, ret=%d\n", pix_fmt, ret);
		return ret;
	}

	m_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	pix.field  = field;
	pix.width  = width;
	pix.height = height;
	pix.pixelformat = pix_fmt_info.pix_fmt;
	pix.colorspace  = pix_fmt_info.clrspc;
	pix.num_planes  = pix_fmt_info.num_planes;

	ret = m_vpeDev->s_fmt(m_fmt);
	if (ret) {
		iray_err("set format fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int VpeOutput::req_buf()
{
	int ret = 0;

	m_req_buf.type   = m_fmt.type;
	m_req_buf.memory = V4L2_MEMORY_MMAP;
	m_req_buf.count  = NUM_OF_SRC_BUFFERS;
	ret = m_vpeDev->req_bufs(m_req_buf);
	if (ret) {
		iray_err("reqeuset buffer fail, ret=%d\n", ret);
		return ret;
	}

	if (m_req_buf.count != NUM_OF_SRC_BUFFERS) {
		iray_warning("NUM_OF_DST_BUFFERS=%d, m_req_buf.count=%d\n", NUM_OF_SRC_BUFFERS, m_req_buf.count);
	}

	return SUCCESS;
}

int VpeOutput::initBuffer(struct v4l2_buffer &buf)
{
	void *planes = NULL;
	int num_planes = m_fmt.fmt.pix_mp.num_planes;

	planes = (struct v4l2_plane *)malloc(sizeof(struct v4l2_plane) * num_planes);
	if (planes == NULL) {
		iray_err("malloc mem for planes fail\n");
		return -ENOMEM;
	}
	
	buf.type     = m_fmt.type;
	buf.memory   = V4L2_MEMORY_MMAP;
	buf.length   = num_planes;
	buf.m.planes = (struct v4l2_plane *)planes;

	return SUCCESS;
}

void VpeOutput::freeBuffer(struct v4l2_buffer &buf)
{
	CHECK_FREE(buf.m.planes);
	buf.length = 0;
}

int VpeOutput::initBufferManager(struct v4l2_requestbuffers &req_buf)
{
	u32 i = 0;
	int ret = 0;
	int mem_size = 0;

	// step 1: alloc memory for buffers
	m_buf_mag.size = req_buf.count;
	mem_size = sizeof(struct v4l2_buffer) * m_buf_mag.size;

	m_buf_mag.bufs = (struct v4l2_buffer *)malloc(mem_size);
	if (m_buf_mag.bufs == NULL) {
		iray_err("malloc mem for buffers fail\n");
		return -ENOMEM;
	}

	memset(m_buf_mag.bufs, 0x00, mem_size);

	// step 2: init memory for every buffer
	for (i =0; i < m_buf_mag.size; i++) {
		ret = initBuffer(m_buf_mag.bufs[i]);
		if (ret) {
			iray_err("initBuffer fail, index=%d\n", i);
			break;
		}

		m_buf_mag.bufs[i].index = i;
	}

	// error handle
	if (i != m_buf_mag.size) {
		freeBufferManager();
		return -ENOMEM;
	}

	return SUCCESS;
}

void VpeOutput::freeBufferManager()
{
	// step 1: free buffers
	for (u32 i = 0; i < m_buf_mag.size; i++) {
		freeBuffer(m_buf_mag.bufs[i]);
	}

	// step 2: free buffers memory
	CHECK_FREE(m_buf_mag.bufs);
	m_buf_mag.size = 0;
}

int VpeOutput::initBufferUser(struct v4l2_buffer_user &buf_usr, int num_planes)
{
	void *planes = NULL;

	planes = (struct v4l2_plane_user *)malloc(sizeof(struct v4l2_plane_user) * num_planes);
	if (planes == NULL) {
		iray_err("malloc mem for user planes fail\n");
		return -ENOMEM;
	}

	buf_usr.is_queued  = FALSE;
	buf_usr.num_planes = num_planes;
	buf_usr.planes     = (struct v4l2_plane_user *)planes;

	return SUCCESS;
}

void VpeOutput::freeBufferUser(struct v4l2_buffer_user &buf_usr)
{
	CHECK_FREE(buf_usr.planes);
	buf_usr.index      = 0;
	buf_usr.is_queued  = FALSE;
	buf_usr.num_planes = 0;
}

int VpeOutput::initBufferManagerUser(struct v4l2_requestbuffers &req_buf)
{
	u32 i = 0;
	int ret = 0;
	int mem_size = 0;

	// step 1:alloc memory for buffer manager
	m_buf_mag_usr.size = req_buf.count;
	mem_size = sizeof(struct v4l2_buffer_user) * m_buf_mag_usr.size;
	m_buf_mag_usr.buf_users = (struct v4l2_buffer_user *)malloc(mem_size);
	if (m_buf_mag_usr.buf_users == NULL) {
		iray_err("malloc mem for user buffers fail\n");
		return -ENOMEM;
	}

	memset(m_buf_mag_usr.buf_users, 0x00, mem_size);

	// step 2: init buffers in buffer manager
	int num_planes = m_fmt.fmt.pix_mp.num_planes;
	for (i = 0; i < m_buf_mag_usr.size; i++) {
		ret = initBufferUser(m_buf_mag_usr.buf_users[i], num_planes);
		if (ret) {
			iray_err("initBufferUser fail, index=%d, ret=%d\n", i, ret);
			break;
		}
	}

	// error handle
	if (i != m_buf_mag_usr.size) {
		freeBufferManagerUser();
		return -ENOMEM;
	}

	return SUCCESS;
}

void VpeOutput::freeBufferManagerUser()
{
	// step 1: free buffer user
	for (u32 i = 0; i < m_buf_mag_usr.size; i++) {
		freeBufferUser(m_buf_mag_usr.buf_users[i]);
	}

	// step 2: delete mem
	CHECK_FREE(m_buf_mag_usr.buf_users);
	m_buf_mag_usr.size = 0;
}

int VpeOutput::query_bufs()
{
	int ret = 0;

	for (u32 i = 0; i < m_buf_mag.size; i++) {

		m_buf_mag.bufs[i].index = i;
		m_buf_mag_usr.buf_users[i].index = i;
		ret = m_vpeDev->query_buf(m_buf_mag.bufs[i]);
		if (ret) {
			iray_err("src query buf fail, ret=%d\n", ret);
			return ret;
		}
	}

	return ret;
}

int VpeOutput::mmap_buffer(struct v4l2_buffer &buf, struct v4l2_buffer_user &buf_user)
{
	u32 i = 0;
	void *mmap_addr = NULL;
	struct v4l2_plane *planes = buf.m.planes;

	for (i = 0; i < buf.length; i++) {
		mmap_addr = m_vpeDev->mmap(planes[i].length, planes[i].m.mem_offset);
		if (MAP_FAILED == mmap_addr) {
			iray_err("mmap fail\n");
			break;
		}

		buf_user.planes[i].addr      = mmap_addr;
		buf_user.planes[i].length    = planes[i].length;
		buf_user.planes[i].is_mapped = TRUE;
	}

	if (i !=  buf.length) {
		munmap_buffer(buf_user);
		return -ENOMEM;
	}

	return SUCCESS;
}

void VpeOutput::munmap_buffer(struct v4l2_buffer_user &buf_user)
{
	for (u32 i = 0; i < buf_user.num_planes; i++) {
		if (buf_user.planes[i].is_mapped) {
			m_vpeDev->munmap(buf_user.planes[i].addr, buf_user.planes[i].length);
			buf_user.planes[i].is_mapped = FALSE;
			buf_user.planes[i].addr = NULL;
		}
	}
}

int VpeOutput::mmap_buffers()
{
	u32 i = 0;
	int ret = 0;

	if (m_buf_mag.size != m_buf_mag_usr.size) {
		return -EPARAME;
	}

	for (i = 0; i < m_buf_mag.size; i++) {
		ret = mmap_buffer(m_buf_mag.bufs[i], m_buf_mag_usr.buf_users[i]);
		if (ret) {
			iray_err("mmap_buffer fail, index=%d\n", i);
			break;
		}
	}

	if (i != m_buf_mag.size) {
		munmap_buffers();
		return ret;
	}

	return SUCCESS;
}

void VpeOutput::munmap_buffers()
{
	for (u32 i = 0; i < m_buf_mag_usr.size; i++) {
		munmap_buffer(m_buf_mag_usr.buf_users[i]);
	}
}

u32 VpeOutput::getAFreeBuffer()
{
	int ret = 0;

	// 1.find a free buffer to save the data
	for (u32 i = 0; i < m_buf_mag_usr.size; i++) {
		if (!m_buf_mag_usr.buf_users[i].is_queued) {
			return i;
		}
	}

	// if not found, dequeue
	ret = m_vpeDev->dq_buf(m_buf_for_dq);
	if (ret) {
		iray_err("dqueue buffer fail, ret=%d\n", ret);
		return -ENOMEM;
	}

	return m_buf_for_dq.index;
}

int VpeOutput::ouput(void *data, u32 len)
{
	int ret = 0;

	// get buffer to save data
	if (m_output_buf == NULL) {
		u32 index = getAFreeBuffer();
		if (index >= 0) {
			m_output_buf = &m_buf_mag_usr.buf_users[index];
		}
	}

	if (m_output_buf == NULL) {
		iray_err("get buffer fail\n");
		return -ENOMEM;
	}

	// check datao info and buffer infor
	struct v4l2_plane_user *plane = NULL;
	plane = &(m_output_buf->planes[m_output_index]);
	if (len != plane->length) {
		iray_err("data len error, len=%d, plane len=%d\n", len, plane->length);
		return -EPARAME;
	}

	// copy and 
	memcpy(plane->addr, data, len);

	m_output_index += 1;
	if (m_output_index < m_output_buf->num_planes) {
		// data not enough
		iray_dbg("data not enough\n");
		return 0;
	}

	// output buffer area full, flush data
	int index = m_output_buf->index;
	m_output_buf->is_queued = TRUE;
	
	m_output_index = 0;
	m_output_buf = NULL;

	// queue
	struct v4l2_plane *planes = m_buf_mag.bufs[index].m.planes;
	memset(planes, 0x00, sizeof(struct v4l2_plane));
	planes[0].length = len;
	planes[0].bytesused = len;
	planes[0].data_offset = 0;
	ret = queue(index);
	if (ret) {
		iray_err("queue buffer fail, ret=%d\n", ret);
		return -ENOMEM;
	}

	return SUCCESS;
}

