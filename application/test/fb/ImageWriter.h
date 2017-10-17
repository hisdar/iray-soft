#ifndef _IMAGE_WRITER_H_
#define _IMAGE_WRITER_H_

#ifndef u32
#define u32 unsigned int
#endif
void saveBitmap(const char *file_path, unsigned char *pData, int width, int height, int nDatasize);


int write_jpeg_file(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality);

#endif
