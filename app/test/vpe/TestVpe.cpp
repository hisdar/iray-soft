#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <IrayRgbImage.h>
#include <Vpe.h>

int main(int argc, char *argv[])
{
	int ret = 0;
	Vpe vpe;

	u32 file_size = 640 * 576 * 2;
	void *file_data = malloc(file_size);
	if (file_data == NULL) {
		printf("alloc mem for file data fail\n");
		return 0;
	}

	u32 output_len = 1280 * 720 * 3;
	char *output_data = (char *)malloc(output_len);
	if (output_data == NULL) {
		printf("alloc mem for output data fail\n");
		return 0;
	}

	memset(output_data, 0x00, output_len);
	
	FILE *file = fopen("./yuyv.bin", "rb+");
	if (file == NULL) {
		printf("open file fail\n");
		return 0;
	}

	ret = fread(file_data, file_size, 1, file);
	if (ret != 1) {
		printf("read file fail\n");
		return 0;
	}

	ret = vpe.init(640, 576, V4L2_PIX_FMT_YUYV, 1280, 720, V4L2_PIX_FMT_RGB24);
	if (ret) {
		printf("vpe init fail\n");
		return 0;
	} else {
		printf("vpe init success\n");
	}

	ret = vpe.put(&file_data, 1);
	if (ret) {
		printf("put data to vpe fail\n");
		return 0;
	} else {
		printf("put data to vpe success\n");
	}

	ret = vpe.get(&output_data, 1);
	if (ret) {
		printf("get data from vpe fail\n");
		return 0;
	} else {
		printf("get data from vpe success\n");
	}

	IrayRgbImage srcImg;
	srcImg.create(640, 576, COLOR_TYPE_RGB);
	srcImg.formatFromYUY2((char *)file_data, 640, 576);
	srcImg.save("rgb-src.bmp", IRAY_IMG_TYPE_BMP);

	IrayRgbImage img;
	ret = img.createFromExternalMem(output_data, 1280, 720, COLOR_TYPE_RGB);
	if (ret) {
		printf("create rgb image fail\n");
		return 0;
	} else {
		printf("create rgb image success\n");
	}
	
	ret = img.save("./rgb-dst.bmp", IRAY_IMG_TYPE_BMP);
	if (ret) {
		printf("save rgb file fail\n");
		return 0;
	}

	printf("success !!!\n");
	return 0;
}

