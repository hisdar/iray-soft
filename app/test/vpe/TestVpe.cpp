#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <IrayRgbImage.h>
#include <Vpe.h>

#define SRC_WIDTH  640
#define SRC_HEIGHT 576

#define DST_WIDTH  1920
#define DST_HEIGHT 1080

int main(int argc, char *argv[])
{
	int ret = 0;
	Vpe vpe;

	u32 file_size = SRC_WIDTH * SRC_HEIGHT * 2;
	void *file_data = malloc(file_size);
	if (file_data == NULL) {
		printf("alloc mem for file data fail\n");
		return 0;
	}

	u32 output_len = DST_WIDTH * DST_HEIGHT * 2;
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

	ret = vpe.init(SRC_WIDTH, SRC_HEIGHT, V4L2_PIX_FMT_YUYV, DST_WIDTH, DST_HEIGHT, V4L2_PIX_FMT_YUYV);
	if (ret) {
		printf("vpe init fail\n");
		return 0;
	} else {
		printf("vpe init success\n");
	}

	for (int i = 0; i < 1; i++) {
		ret = vpe.put(file_data, file_size);
		if (ret) {
			printf("vpe put data fail\n");
			return 0;
		}

		ret = vpe.get(output_data, output_len);
		if (ret) {
			printf("vpe put data fail\n");
			return 0;
		}
	}

	/*IrayRgbImage srcImg;
	srcImg.create(640, 576, COLOR_TYPE_RGB);
	srcImg.formatFromYUY2((char *)file_data, 640, 576);
	srcImg.save("rgb-src.bmp", IRAY_IMG_TYPE_BMP);

	IrayRgbImage img;
	ret = img.createFromExternalMem(output_data, 1920, 1080, COLOR_TYPE_RGB);
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

	free(file_data);
	free(output_data);*/

	printf("success !!!\n");
	return 0;
}

