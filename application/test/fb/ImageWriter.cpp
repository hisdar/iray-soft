#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <setjmp.h>  
#include <math.h>  
#include <sys/time.h>  
#include <time.h>  
  
// jpeg库头文件必须放到stdio.h后面  
#include "jpeglib.h"  
#include "jerror.h"  

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
  
void saveBitmap(const char *file_path, unsigned char *pData, int width, int height, int nDatasize)  
{
    // Define BMP Size
    const int size = nDatasize;
  
    // Part.1 Create Bitmap File Header  
    BITMAPFILEHEADER fileHeader;
	
    fileHeader.bfType = 0x4D42;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Part.2 Create Bitmap Info Header
    BITMAPINFOHEADER bitmapHeader = { 0 };

    bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapHeader.biHeight = -height;
    bitmapHeader.biWidth = width;
    bitmapHeader.biPlanes = 1;
    bitmapHeader.biBitCount = 24;
    bitmapHeader.biSizeImage = size;
    bitmapHeader.biCompression = 0; //BI_RGB

    // Write to file
    FILE *output = fopen(file_path, "wb");
    if (output == NULL)
    {
        printf("Cannot open file!\n");
    }
    else
    {
        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, output);
        fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, output);
        fwrite(pData, size, 1, output);
		fflush(output);
        fclose(output);
    }
}


// the range of quality is 0 ~ 100
int write_jpeg_file(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality)  
{  
    struct jpeg_compress_struct cinfo;  
    struct jpeg_error_mgr jerr;  
    int row_stride = 0;  
    FILE* fp = NULL;  
    JSAMPROW row_pointer[1];  
  
    cinfo.err = jpeg_std_error(&jerr);  
  
    jpeg_create_compress(&cinfo);  
    fp = fopen(jpeg_file, "wb");  
    if (fp == NULL)  
    {  
        printf("open file %s failed.\n", jpeg_file);  
        return -1;  
    }  
    jpeg_stdio_dest(&cinfo, fp);  
    cinfo.image_width = width;  
    cinfo.image_height = height;  
    cinfo.input_components = 3;  
    cinfo.in_color_space = JCS_RGB;  
  
    jpeg_set_defaults(&cinfo);  
    jpeg_set_quality(&cinfo, quality, 1);  // todo 1 == true  
    jpeg_start_compress(&cinfo, TRUE);  
    row_stride = width * cinfo.input_components;  
  
    while (cinfo.next_scanline < cinfo.image_height)  
    {  
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];  
        jpeg_write_scanlines(&cinfo, row_pointer, 1);  
    }  
  
    jpeg_finish_compress(&cinfo);  
    jpeg_destroy_compress(&cinfo);  
    fclose(fp);  
  
    return 0;  
}  

int read_uyvy_file(const char *file_path, char *data, int data_len)
{
	size_t read_count = 0;
	char read_buf[1024];
	char val_buf[3] = {0};
	int val_idx = 0;
	int data_idx = 0;
	unsigned int hex_buf;
	FILE *fp = NULL;

	fp = fopen(file_path, "r+");
	if (fp == NULL) {
		printf("open file fail\n");
		return -1;
	}

	read_count = fread(read_buf, 1, 1024, fp);
	while (read_count > 0) {

		for (size_t i = 0; i < read_count; i++) {
			if (!((read_buf[i] >= '0' && read_buf[i] <= '9')
				|| (read_buf[i] >= 'a' && read_buf[i] <= 'f')
				|| (read_buf[i] >= 'A' && read_buf[i] <= 'F')))
			{
				continue;
			}

			val_buf[val_idx] = read_buf[i];
			val_idx += 1;
			if (val_idx == 2) {
				val_idx = 0;
				sscanf(val_buf, "%x", &hex_buf);
				data[data_idx] = (char)hex_buf;
				data_idx += 1;
				if (data_idx >= data_len) {
					goto exit;
				}
			}
		}

		read_count = fread(read_buf, 1, 1024, fp);
	}
	
exit:
	fclose(fp);
	
	return 0;
}

