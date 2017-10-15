#ifndef _IRAY_RGB_IMAGE_H_
#define _IRAY_RGB_IMAGE_H_

#include <string.h>
#include "Bt656Merge.h"

#define CHECK_FREE(ptr) \
	if (ptr != NULL) { \
		free(ptr); \
		ptr = NULL; \
	}

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u8
#define u8 unsigned char
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct iray_rgb {
	u8 alpha;
	u8 red;
	u8 green;
	u8 blue;
} RGB;

typedef enum iray_color_type {
	COLOR_TYPE_RGB,
	COLOR_TYPE_ARGB,	
} COLOR_TYPE;

typedef enum iray_img_type {
	IRAY_IMG_TYPE_BMP,
	IRAY_IMG_TYPE_JPG,
	
} IMG_TYPE;

class IrayRgbImage {
public:
	IrayRgbImage();
	IrayRgbImage(u32 width, u32 height, COLOR_TYPE color_type);
	~IrayRgbImage();

	int create(u32 width, u32 height, COLOR_TYPE color_type);
	int createFromExternalMem(char *external_mem_addr, u32 width, u32 height, COLOR_TYPE color_type);
	int createFromUYVY(char * src, u32 width, u32 height, COLOR_TYPE color_type);
	int createFromYUY2(char *src, u32 width, u32 height, COLOR_TYPE color_type);

	int draw(IrayRgbImage *img, u32 x, u32 y);
	int save(const char *path, IMG_TYPE img_type);
	
	char *getData();

	u32 getLength();
	u32 getWidth();
	u32 getHeight();
	u32 getBytesPerPix();

private:
	char *m_argb_buf;

	u32 m_r_idx;
	u32 m_g_idx;
	u32 m_b_idx;
	u32 m_a_idx;
	
	u32 m_width;
	u32 m_height;
	
	u8 m_bytes_per_pix;

	void yuvTorgb(u8 Y, u8 U, u8 V, RGB * rgb);
	void setColor(char * addr, RGB * rgb);
	int init(u32 width, u32 height, COLOR_TYPE color_type, u8 is_alloc_mem);
	int saveBitMap(const char * file_path);
};

#endif
