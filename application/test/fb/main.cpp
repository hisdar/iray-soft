#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include<sys/time.h>

#include "FrameBufferDev.h"

#include "ImageWriter.h"

#include "Bt656Merge.h"
#include "IrayRgbImage.h"

int test_all()
{
	int ret = 0;
	char *top_img = NULL;
	char *bottom_img = NULL;
	int data_len = 0;

	data_len = 576 * 640 * 2;
	top_img = (char *)malloc(data_len);
	bottom_img = (char *)malloc(data_len);

	// load file data
	ret = read_uyvy_file("/home/caowei/temp/image-8.ihex", top_img, data_len);
	ret |= read_uyvy_file("/home/caowei/temp/image-8.ihex", bottom_img, data_len);
	if (ret) {
		printf("read src file fail, ret=%d\n", ret);
		return ret;
	} else {
		printf("read src file success\n");
	}

	struct timeval start_tv, end_tv;
	gettimeofday(&start_tv, NULL);

	// merge two bt656 image
	Bt656Merge bt656Merge;
	struct iray_bt656_format fmt;
	fmt.width = 640;
	fmt.height = 576;
	fmt.bytes_per_pix = 2;
	fmt.field_type = FIELD_TYPE_OE;
	
	ret = bt656Merge.init(&fmt);
	if (ret) {
		printf("bt656 init fail\n");
		return ret;
	} else {
		printf("bt656 init success\n");
	}

	bt656Merge.merge(top_img, FIELD_TOP);
	ret = bt656Merge.merge(bottom_img, FIELD_BOTTOM);
	if (ret) {
		printf("merge image fail\n");
		return ret;
	} else {
		printf("merge image success\n");
	}

	// create rgb image / yuv --> rgb		
	IrayRgbImage rgbImage;
    rgbImage.create(640, 576, COLOR_TYPE_RGB);
	ret = rgbImage.formatFromYUY2(bt656Merge.getData(), 640, 576);
	if (ret) {
		printf("create rgb image fail\n");
		return ret;
	} else {
		printf("create rgb image success\n");
	}

	IrayRgbImage backImage;
	backImage.create(1920, 1080, COLOR_TYPE_RGB);
	backImage.draw(&rgbImage, 1480, 500);

	gettimeofday(&end_tv, NULL);
	printf("time[%lu.%lu]\n", end_tv.tv_sec - start_tv.tv_sec, end_tv.tv_usec - start_tv.tv_usec);

	backImage.save("./all.bmp", IRAY_IMG_TYPE_BMP);
	gettimeofday(&start_tv, NULL);

	printf("time[%lu.%lu]\n", start_tv.tv_sec - end_tv.tv_sec, start_tv.tv_usec - end_tv.tv_usec);
	
	free(top_img);
	free(bottom_img);

	return 0;
}

int main(int argc, char *argv[])
{
	/*int ret = 0;
	FrameBufferDev fb_dev;

	ret = fb_dev.open("/dev/fb0");
	if (ret) {
		printf("open device fail\n");
		return ret;
	}*/

	//fb_dev.showScreenInfo();

	//fb_dev.prepareOutput();
	//fb_dev.outputImage(NULL, 0, 0, 0);
	/*unsigned char *img_buff = NULL;

	unsigned int img_width = 800;
	unsigned int img_height = 600;
	img_buff = (unsigned char *)malloc(3 * img_width * img_height);

	memset(img_buff, 255, img_width * img_height);
	memset(img_buff + img_width * img_height, 100, img_width * img_height);
	memset(img_buff + img_width * img_height * 2, 200, img_width * img_height);
	write_jpeg_file("./test.jpg", img_buff, img_width, img_height, 0);*/

	test_all() ;
	
	return 0;
}
