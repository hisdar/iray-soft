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

int FrameBufferDev::prepareOutput()
{
	int ret = 0;

	ret = queryScreenInfo();
	if (ret) {
		return ret;
	}

	m_fb_buf_len = m_fix.line_length * m_var.yres;
	m_fb_mem_addr = mmap(NULL, m_fb_buf_len, PROT_READ | PROT_WRITE, MAP_SHARED, m_fb, 0);
	if (m_fb_mem_addr == MAP_FAILED) {
		printf("mmap failed\n");
	}

	return 0;
}

int FrameBufferDev::outputImage(char *data, u32 width, u32 height, u32 bytes_per_pix)
{

	while (1) {
		memset(m_fb_mem_addr, 0x00, m_fb_buf_len);
	}
	return 0;
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
	printf("pixclock      :[%u]\n", m_var.pixclock);
	printf("margin        :[%u, %u, %u, %u]\n", m_var.left_margin, m_var.upper_margin, m_var.right_margin, m_var.lower_margin);
	printf("sync_len      :[%u, %u]\n", m_var.hsync_len, m_var.vsync_len);
	printf("sync          :[%u]\n", m_var.sync);
	printf("vmode         :[%u]\n", m_var.vmode);
	printf("rotate        :[%u]\n", m_var.rotate);
	printf("colorspace    :[%u]\n", m_var.colorspace);

	printf("---------- fix info ----------\n");
	printf("colorspace    :[%s]\n", m_fix.id);
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

	return 0;
}
	