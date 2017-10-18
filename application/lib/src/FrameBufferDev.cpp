#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <linux/omapfb.h>
#include <linux/fb.h>
#include <stdio.h>
#include <sys/types.h>  
#include "FrameBufferDev.h"

FrameBufferDev::FrameBufferDev()
{
	m_fb = 0;
	m_fb_buf_len = 0;
	m_fb_mem_addr = NULL;
}

FrameBufferDev::~FrameBufferDev()
{
	::close(m_fb);
}

int FrameBufferDev::open(const char *dev_path)
{
	m_fb = ::open(dev_path, O_RDWR);
	if (m_fb <= 0) {
		return -ENODEV;
	}

	return SUCCESS;
}

int FrameBufferDev::setFrameSize(u32 width, u32 height)
{
	int ret = 0;
	queryScreenInfo();

	m_var.xres = width;
	m_var.yres = height;
	m_var.xres_virtual = width;
	m_var.yres_virtual = height;

	ret = ioctl(m_fb, FBIOPUT_VSCREENINFO, &m_var);
	if (ret < 0) {
		printf("Error set variable information, ret=%d.\n", ret);
		return ret;
	}

	return SUCCESS;
}

int FrameBufferDev::prepareOutput()
{
	int ret = 0;

	ret = queryScreenInfo();
	if (ret) {
		return ret;
	}

	// setFrameSize(1280, 720);

	m_fb_buf_len = m_fix.line_length * m_var.yres;// + 128 * m_var.bits_per_pixel;
	m_fb_mem_addr = (char *)mmap(NULL, m_fb_buf_len, PROT_READ | PROT_WRITE, MAP_SHARED, m_fb, 0);
	if (m_fb_mem_addr == MAP_FAILED) {
		iray_err("mmap failed\n");
		return -ENOMEM;
	}

	// 128 pix for hide
	//ret = m_fb_img.create(m_var.xres + 128, m_var.yres, COLOR_TYPE_ARGB);
	ret = m_fb_img.createFromExternalMem(m_fb_mem_addr, m_var.xres + 128, m_var.yres, COLOR_TYPE_ARGB);
	if (ret ) {
		iray_err("m_fb_img.create fail, ret = %d, size[%u, %u]\n",
			ret, m_var.xres, m_var.yres);
		return ret;
	}

	return 0;
}

int FrameBufferDev::outputImage(IrayRgbImage *img, int x, int y)
{
	int ret = 0;

	ret = m_fb_img.draw(img, x, y);
	if (ret) {
		iray_err("m_fb_img draw fail, ret=%d\n", ret);
		return ret;
	}

	//memcpy(m_fb_mem_addr, m_fb_img.getData(), m_fb_img.getLength());

	return SUCCESS;
}

int FrameBufferDev::queryScreenInfo()
{
	int ret = 0;

	memset(&m_var, 0x00, sizeof(m_var));
	ret = ioctl(m_fb, FBIOGET_VSCREENINFO, &m_var);
	if (ret < 0) {
		printf("Error reading variable information, ret=%d.\n", ret);
		return ret;
	}

	memset(&m_fix, 0x00, sizeof(m_fix));
	ret = ioctl(m_fb, FBIOGET_FSCREENINFO, &m_fix);
	if(ret < 0) {
		printf("Cannot get fix screen information, ret=%d\n", ret);
		return ret;
	}

	memset(&m_con, 0x00, sizeof(m_con));
	ret = ioctl(m_fb, FBIOGET_CON2FBMAP, &m_con);
	if(ret < 0) {
		printf("Cannot get console screen information, ret=%d\n", ret);
		//return ret;
	}

	struct omapfb_plane_info pi;
	memset(&pi, 0x00, sizeof(pi));
	ret = ioctl(m_fb, OMAPFB_QUERY_PLANE, &pi);
	if(ret < 0) {
		printf("Cannot get omapfb_plane_info information, ret=%d\n", ret);
		//return ret;
	}

	return 0;
}

int FrameBufferDev::showScreenInfo()
{
	int ret = 0;

	ret = queryScreenInfo();
	if (ret) {
		return ret;
	}

	printf("---------- var info ----------\n");
	printf("res           :[%u, %u]\n", m_var.xres, m_var.yres);
	printf("res_virtual   :[%u, %u]\n", m_var.xres_virtual, m_var.yres_virtual);
	printf("res_offset    :[%u, %u]\n", m_var.xoffset, m_var.yoffset);
	printf("bits_per_pixel:[%u]\n", m_var.bits_per_pixel);
	printf("grayscale     :[%u]\n", m_var.grayscale);
	printf("bit-field-r   :[%u, %u, %u]\n", m_var.red.length, m_var.red.msb_right, m_var.red.offset);
	printf("bit-field-g   :[%u, %u, %u]\n", m_var.green.length, m_var.green.msb_right, m_var.green.offset);
	printf("bit-field-b   :[%u, %u, %u]\n", m_var.blue.length, m_var.blue.msb_right, m_var.blue.offset);
	printf("bit-field-t   :[%u, %u, %u]\n", m_var.transp.length, m_var.transp.msb_right, m_var.transp.offset);
	printf("nonstd        :[%u]\n", m_var.nonstd);
	printf("activate      :[%u]\n", m_var.activate);
	printf("size          :[%u, %u]\n", m_var.width, m_var.height);
	printf("accel_flags   :[%u]\n", m_var.accel_flags);

	// 
	printf("pixclock      :[%u]\n", m_var.pixclock);
	printf("margin        :[%u, %u, %u, %u]\n", m_var.left_margin, m_var.upper_margin, m_var.right_margin, m_var.lower_margin);
	printf("sync_len      :[%u, %u]\n", m_var.hsync_len, m_var.vsync_len);
	printf("sync          :[%u]\n", m_var.sync);
	printf("vmode         :[%u]\n", m_var.vmode);
	printf("rotate        :[%u]\n", m_var.rotate);
	printf("colorspace    :[%u]\n", m_var.colorspace);

	printf("---------- fix info ----------\n");
	printf("id            :[%s]\n", m_fix.id);
	printf("smem_start    :[%lu]\n", m_fix.smem_start);
	printf("smem_len      :[%u]\n", m_fix.smem_len);
	printf("type          :[%u]\n", m_fix.type);
	printf("type_aux      :[%u]\n", m_fix.type_aux);
	printf("visual        :[%u]\n", m_fix.visual);
	printf("xpanstep      :[%u]\n", m_fix.xpanstep);
	printf("ypanstep      :[%u]\n", m_fix.ypanstep);
	printf("ywrapstep     :[%u]\n", m_fix.ywrapstep);
	printf("line_length   :[%u]\n", m_fix.line_length);
	printf("mmio_start    :[%lu]\n", m_fix.mmio_start);
	printf("mmio_len      :[%u]\n", m_fix.mmio_len);
	printf("accel         :[%u]\n", m_fix.accel);
	printf("capabilities  :[%u]\n", m_fix.capabilities);

	printf("---------- console to fb info ----------\n");
	printf("console       :[%u]\n", m_con.console);
	printf("framebuffer   :[%u]\n", m_con.framebuffer);

	return 0;
}


int FrameBufferDev::receiveFrame(IrayCameraData *frameData)
{
	int ret = 0;
	

	char *src = frameData->getAddr();
	u32 width = frameData->getImageWidth();
	u32 height = frameData->getImageHeight();
	
	//iray_dbg("image size[%d, %d], addr[0x%X]\n", width, height, (u32)src);
	if (width != m_src_img.getHeight() || height != m_src_img.getHeight()) {
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
	}

	return outputImage(&m_src_img, 640, 252);
}