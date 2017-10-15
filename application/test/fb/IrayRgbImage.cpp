#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#include "IrayRgbImage.h"

#define PACKED __attribute__( ( packed, aligned(1) ) )

typedef unsigned char  BYTE;  
typedef unsigned short WORD;
typedef unsigned int  DWORD;

typedef struct {  
    WORD    bfType;      // 2
    DWORD   bfSize;      // 4
    WORD    bfReserved1; // 2 
    WORD    bfReserved2; // 2
    DWORD   bfOffBits;   // 4
}PACKED BITMAPFILEHEADER;  
  
typedef struct {  
    DWORD      biSize;         // 4
    DWORD      biWidth;        // 4
    DWORD      biHeight;       // 4
    WORD       biPlanes;       // 2
    WORD       biBitCount;     // 2
    DWORD      biCompression;  // 4
    DWORD      biSizeImage;    // 4
    DWORD      biXPelsPerMeter;// 4
    DWORD      biYPelsPerMeter;// 4
    DWORD      biClrUsed;      // 4
    DWORD      biClrImportant; // 4 
}PACKED BITMAPINFOHEADER;  

IrayRgbImage::IrayRgbImage()
{
	m_argb_buf = NULL;
}

IrayRgbImage::IrayRgbImage(u32 width, u32 height, COLOR_TYPE color_type)
{
	m_argb_buf = NULL;
	init(width, height, color_type, TRUE);
}

IrayRgbImage::~IrayRgbImage()
{
	CHECK_FREE(m_argb_buf);
}

int IrayRgbImage::init(u32 width, u32 height, COLOR_TYPE color_type, u8 is_alloc_mem)
{
	m_width = width;
	m_height = height;
	
	if (color_type == COLOR_TYPE_ARGB) {
		m_bytes_per_pix = 4;
		m_a_idx = 0;
		m_r_idx = 1;
		m_g_idx = 2;
		m_b_idx = 3;
	} else if (color_type == COLOR_TYPE_RGB) {
		m_bytes_per_pix = 3;
		m_a_idx = 0;
		m_r_idx = 0;
		m_g_idx = 1;
		m_b_idx = 2;

	} else {
		printf("bad color type\n");
		return -1;
	}

	CHECK_FREE(m_argb_buf);
	if (is_alloc_mem) {
		int mem_len = width * height * m_bytes_per_pix;
		m_argb_buf = (char *)malloc(mem_len);
		if (m_argb_buf == NULL) {
			printf("alloc mem fail\n");
			return -1;
		}

		memset(m_argb_buf, 0x00, mem_len);
	}

	return 0;
}

int IrayRgbImage::create(u32 width, u32 height, COLOR_TYPE color_type)
{
	return init(width, height, color_type, TRUE);
}

int IrayRgbImage::createFromExternalMem(char *external_mem_addr, u32 width, u32 height, COLOR_TYPE color_type)
{
	int ret = 0;

	ret = init(width, height, color_type, FALSE);
	if (ret) {
		return ret;
	}

	m_argb_buf = external_mem_addr;

	return 0;
}

int IrayRgbImage::createFromUYVY(char *src, u32 width, u32 height, COLOR_TYPE color_type)
{
	int ret = 0;
	RGB pixRgb;
	
	ret = init(width, height, color_type, TRUE);
	if (ret) {
		printf("init fail\n");
		return ret;
	}

    int line_width = 2 * width;
	pixRgb.alpha = 0;
    for (u32 i = 0; i < height; i++){
        int startU = i * line_width;

		for(int j = 0; j < line_width; j += 4){
			int U  = j + startU;  
            int Y1 = U + 1;  
            int Y2 = U + 3;  
            int V  = U + 2;

			int index = (U >> 1) * m_bytes_per_pix;
            yuvTorgb(src[Y1], src[U], src[V], &pixRgb);
			setColor(&m_argb_buf[index], &pixRgb);

            index += m_bytes_per_pix;
            yuvTorgb(src[Y2], src[U], src[V], &pixRgb);
			setColor(&m_argb_buf[index], &pixRgb);			
        }
    }  

	return 0;
}

int IrayRgbImage::createFromYUY2(char *src, u32 width, u32 height, COLOR_TYPE color_type)
{

	int ret = 0;
	RGB pixRgb;
	
	ret = init(width, height, color_type, TRUE);
	if (ret) {
		printf("init fail\n");
		return ret;
	}

	int line_width = 2 * width;
	pixRgb.alpha = 0;
	for (u32 i = 0; i < height; i++){
		int startY = i * line_width;

		for(int j = 0; j < line_width; j += 4){
			//printf("[%d, %d]\n", i, j);
			int Y1 = j + startY;
			int Y2 = Y1 + 2;
			int U  = Y1 + 1;
			int V  = Y1 + 3;

			int index = (Y1 >> 1) * m_bytes_per_pix;
			yuvTorgb(src[Y1], src[U], src[V], &pixRgb);
			setColor(&m_argb_buf[index], &pixRgb);

			index += m_bytes_per_pix;
			yuvTorgb(src[Y2], src[U], src[V], &pixRgb);
			setColor(&m_argb_buf[index], &pixRgb);			
		}
	} 
	
    return 0;
}

int IrayRgbImage::draw(IrayRgbImage *img, u32 x, u32 y)
{
	if (x > m_width || y > m_height) {
		printf("position out of bounds:width[%u], height[%u], x[%u], y[%u]\n", m_width, m_height, x, y);
		return -1;
	}

	if (m_bytes_per_pix != img->getBytesPerPix()) {
		printf("data format not match:cur[%u], src[%u]\n", m_bytes_per_pix, img->getBytesPerPix());
		return -2;
	}

	u32 marge_left = x * m_bytes_per_pix;	
	u32 tag_line_len = m_width * m_bytes_per_pix;
	u32 src_line_len = img->getWidth() * img->getBytesPerPix();

	u32 copy_line_len = src_line_len;
	if (tag_line_len < copy_line_len + marge_left) {
		copy_line_len = tag_line_len - marge_left;
	}

	u32 copy_height = img->getHeight();
	if (m_height < copy_height + y) {
		copy_height = m_height - y;
	}

	char *tag_addr = m_argb_buf + y * tag_line_len + x * m_bytes_per_pix;
	char *src_addr = img->getData();
	for (u32 i = 0; i < copy_height; i++) {
		memcpy(tag_addr, src_addr, copy_line_len);
		tag_addr += tag_line_len;
		src_addr += src_line_len;
	}

	return 0;
}

int IrayRgbImage::save(const char *file_path, IMG_TYPE img_type)
{
	switch(img_type) {

		case IRAY_IMG_TYPE_BMP:
			return saveBitMap(file_path);
		default:
			printf("un matched image type[%u]\n", img_type);
	}

	return -1;
}

int IrayRgbImage::saveBitMap(const char *file_path)  
{
    // Define BMP Size
    const int size = getLength();
  
    // Part.1 Create Bitmap File Header  
    BITMAPFILEHEADER file_header;
	
    file_header.bfType      = 0x4D42;  // char:BM
    file_header.bfReserved1 = 0;
    file_header.bfReserved2 = 0;
    file_header.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size;
    file_header.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Part.2 Create Bitmap Info Header
    BITMAPINFOHEADER bitmap_header = {0};

    bitmap_header.biSize        = sizeof(BITMAPINFOHEADER);
    bitmap_header.biHeight      = -m_height;
    bitmap_header.biWidth       = m_width;
    bitmap_header.biPlanes      = 1;
    bitmap_header.biBitCount    = m_bytes_per_pix * 8;
    bitmap_header.biSizeImage   = size;
    bitmap_header.biCompression = 0;  //do not compress

    // Part.3 Write to file
    FILE *file = fopen(file_path, "wb");
    if (file == NULL)
    {
        printf("Cannot open file:%s\n", file_path);
		return -1;
    }

    fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&bitmap_header, sizeof(BITMAPINFOHEADER), 1, file);
    fwrite(m_argb_buf, size, 1, file);
	fflush(file);
    fclose(file);

	return 0;
}

void IrayRgbImage::yuvTorgb(u8 Y, u8 U, u8 V, RGB *rgb)
{

    int r = 0, g = 0, b = 0;
	
	r = (int)((Y & 0xff) + 1.4075 * ((V & 0xff) - 128));  
    g = (int)((Y & 0xff) - 0.3455 * ((U & 0xff) - 128) - 0.7169 * ((V & 0xff) - 128));  
    b = (int)((Y & 0xff) + 1.779  * ((U & 0xff) - 128));  

    rgb->red   = (u8)(r < 0 ? 0 : r > 255 ? 255 : r);  
    rgb->green = (u8)(g < 0 ? 0 : g > 255 ? 255 : g);  
    rgb->blue  = (u8)(b < 0 ? 0 : b > 255 ? 255 : b);
}

void IrayRgbImage::setColor(char *addr, RGB *rgb)
{
	if (m_bytes_per_pix == 4) {
		addr[m_a_idx] = 0;
	} else {
		addr[m_a_idx] = rgb->alpha;
	}
	
    addr[m_r_idx] = rgb->red;
    addr[m_g_idx] = rgb->green;
    addr[m_b_idx] = rgb->blue;
}

char * IrayRgbImage::getData()
{
	return m_argb_buf;
}

u32 IrayRgbImage::getLength()
{
	return m_width * m_height * m_bytes_per_pix;
}

u32 IrayRgbImage::getWidth()
{
	return m_width;
}

u32 IrayRgbImage::getHeight()
{
	return m_height;
}

u32 IrayRgbImage::getBytesPerPix()
{
	return m_bytes_per_pix;
}

