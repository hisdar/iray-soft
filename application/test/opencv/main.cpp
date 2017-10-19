#include <stdio.h>
#include <Bt656Merge.h>
#include <opencv2/core/core.hpp>
#include <opencv2/core/ocl.hpp>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


#include <memory.h>
#include <malloc.h>
#include <sys/time.h>


#include "FrameBufferDev.h"

#include "Bt656Merge.h"
#include "IrayRgbImage.h"

using namespace cv;

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

int test_open_cv_resize()
{
	int ret = 0;
	char *top_img = NULL;
	char *bottom_img = NULL;
	int data_len = 0;

	data_len = 576 * 640 * 2;
	top_img = (char *)malloc(data_len);
	bottom_img = (char *)malloc(data_len);

	// load file data
	ret = read_uyvy_file("./image-8.ihex", top_img, data_len);
	ret |= read_uyvy_file("./image-8.ihex", bottom_img, data_len);
	if (ret) {
		printf("read src file fail, ret=%d\n", ret);
		return ret;
	} else {
		printf("read src file success\n");
	}

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

	

	return 0;
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

	ret = rgbImage.save("./rgb-img.bmp", IRAY_IMG_TYPE_BMP);
	ocl::useOpenCL();

	//Sleep(2);
	Size srcSize(576, 640);
	Size dstSize(1080, 1920);

	Mat dstImg(dstSize, CV_8UC3);

	struct timeval start_tv, end_tv;
	gettimeofday(&start_tv, NULL);

	Mat srcImg(srcSize, CV_8UC3, rgbImage.getData());

	double fx = 1.0 * dstSize.width / srcSize.width;
	double fy = 1.0 * dstSize.height / srcSize.height;
	if (fy < fx) {
		fx = fy;
	} else {
		fy = fx;
	}

	resize(srcImg, dstImg, dstSize, fx, fy, INTER_LINEAR);
	gettimeofday(&end_tv, NULL);

	unsigned long diff_usec = end_tv.tv_usec - start_tv.tv_usec;
	unsigned long diff_sec = end_tv.tv_sec - start_tv.tv_sec;
	if (diff_usec < 0) {
		diff_sec -= 1;
		diff_usec += 1000000;
	}

	printf("time[%lu.%lu]\n",diff_sec, diff_usec);

	imwrite("./opencv-save-src.bmp", srcImg);

	imwrite("./opencv-save-dst.bmp", dstImg);

	imwrite("./opencv-save-dst.jpg", dstImg);

	return 0;

}

int save_yuyv(const char *dst_file, char *data, u32 data_len)
{
	FILE *file = NULL;

	file = fopen(dst_file, "rb+");
	if (file == NULL) {
		printf("open file fail\n");
		return -1;
	}

	int ret = fwrite(data, data_len, 1, file);
	if (ret != 1) {
		printf("write file fail\n");
		return -2;
	}
	fflush(file);
	fclose(file);
}

int test_save_bmp_file()
{
	// 1 read bin file to mem

	IrayRgbImage img;
	int ret = img.create(1280, 720, COLOR_TYPE_RGB);
	if (ret) {
		printf("create img fail, ret=%d\n", ret);
		return 0;
	}

	FILE *file = NULL;
	file = fopen("./1280_720p.rgb24", "rb+");
	if (file == NULL) {
		printf("open src file fail\n");
		return 0;
	}

	ret = fread(img.getData(), img.getLength(), 1, file);
	if (ret != 1) {
		printf("read file fail\n");
		return 0;
	}

	ret = img.save("./swap.bmp", IRAY_IMG_TYPE_BMP);
	if (ret) {
		printf("save bmp file fail\n");
		return 0;
	}

	printf("save bmp file success\n");
	return 0;
}

int test_save_bin_file()
{
	int ret = 0;
	char *top_img = NULL;
	char *bottom_img = NULL;
	int data_len = 0;

	data_len = 576 * 640 * 2;
	top_img = (char *)malloc(data_len);
	bottom_img = (char *)malloc(data_len);

	// load file data
	ret = read_uyvy_file("./image-8.ihex", top_img, data_len);
	ret |= read_uyvy_file("./image-8.ihex", bottom_img, data_len);
	if (ret) {
		printf("read src file fail, ret=%d\n", ret);
		return ret;
	} else {
		printf("read src file success\n");
	}

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

	save_yuyv("./yuv.bin", bt656Merge.getData(), data_len);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argv[1][0] == '1') {
		test_save_bmp_file();
	} else {
		test_save_bin_file();
	}
}
