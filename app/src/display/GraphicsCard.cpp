#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <common/base-def.h>
#include <GraphicsCard.h>


GraphicsCard::GraphicsCard()
{
	m_src_img.create(640, 576, COLOR_TYPE_ARGB);
}

GraphicsCard::~GraphicsCard()
{

}

int GraphicsCard::init()
{
	int ret = 0;

	// open the DRM device
	m_dev = drm_open_dev_dumb(GRAPHICS_CARD_PATH);

	// Prepare all connectors and CRTCs
	modeset_prepare(m_dev, &m_modeset_list);

	// Allocate buffers
	modeset_alloc_fbs(m_modeset_list, 2);

	// Allocate private data
	for_each_output(out, m_modeset_list)
		out->data = calloc(1, sizeof(struct flip_data));

	find_planes(m_dev, m_modeset_list);

	for_each_output(out, m_modeset_list) {
		struct flip_data *pdata = (struct flip_data *)out->data;

		drm_create_dumb_fb2(out->fd,
			//out->mode.hdisplay, out->mode.vdisplay,
			640, 576,
			DRM_FORMAT_YUYV,
			&pdata->plane_buf);

		drm_draw_test_pattern(&pdata->plane_buf, 3);

		pdata->w = out->mode.hdisplay;
		pdata->h = out->mode.vdisplay;
	}

	// Set modes
	modeset_set_modes(m_modeset_list);

	return ret;
}

void GraphicsCard::find_planes(int fd, struct modeset_out *modeset_list)
{
	for_each_output(out, modeset_list) {
		struct flip_data *pdata = (struct flip_data *)out->data;

		uint32_t plane_id = drm_reserve_plane(fd);
		ASSERT(plane_id > 0);

		pdata->plane_id = plane_id;

		printf("Output %d: using plane %d\n", out->output_id, plane_id);
	}
}

int GraphicsCard::receiveFrame(IrayCameraData *frameData)
{
	//int ret = 0;
	
	char *src = frameData->getAddr();
	//u32 width = frameData->getImageWidth();
	//u32 height = frameData->getImageHeight();

	if (src == NULL || frameData->getLength() == 0) {
		iray_err("frame data is empty\n");
		return 0;
	}
	
	//iray_dbg("image size[%d, %d], addr[0x%X]\n", width, height, (u32)src);
	/*if (width != m_src_img.getHeight() || height != m_src_img.getHeight()) {
		m_src_img.release();
		ret = m_src_img.create(width, height, COLOR_TYPE_ARGB);
	}

	if (ret) {
		iray_err("image create fail, ret=%d\n", ret);
		return ret;
	}

	ret = m_src_img.formatFromYUY2(src, width, height);
	if (ret) {
		iray_err("img.formatFromYUY2 fail, ret = %d\n", ret);
		return ret;
	}*/

	for_each_output(out, m_modeset_list) {
		struct flip_data *pdata = (struct flip_data *)out->data;
		struct framebuffer *buf;
		int r;

		buf = &pdata->plane_buf;
		//memcpy(buf->planes[0].map, m_src_img.getData(),buf->planes[0].size);
		//memset(buf->planes[0].map, )
		iray_info("panel size=%d, src size=%d\n", buf->planes[0].size, frameData->getLength());
		memcpy(buf->planes[0].map, src, buf->planes[0].size);

		r = drmModeSetPlane(out->fd, pdata->plane_id, out->crtc_id,
			buf->fb_id, 0,
			0, 0, pdata->w, pdata->h,
			0 << 16, 0 << 16,
			buf->width << 16, buf->height << 16);
		ASSERT(r == 0);
	}

	return 0;
}

