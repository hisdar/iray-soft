#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include<sys/time.h>

#include "FrameBufferDev.h"

#include "ImageWriter.h"

#include "Bt656Merge.h"
#include "IrayRgbImage.h"

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


int test_all(int argc, char *argv[])
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
    rgbImage.create(640, 576, COLOR_TYPE_ARGB);
    ret = rgbImage.formatFromYUY2(bt656Merge.getData(), 640, 576);
    if (ret) {
        printf("create rgb image fail\n");
        return ret;
    } else {
        printf("create rgb image success\n");
    }

    IrayRgbImage backImage;
    backImage.create(1920, 1080, COLOR_TYPE_ARGB);
    backImage.draw(&rgbImage, 640, 252);

    gettimeofday(&end_tv, NULL);
    printf("time[%lu.%lu]\n", end_tv.tv_sec - start_tv.tv_sec, end_tv.tv_usec - start_tv.tv_usec);

    backImage.save("./all.bmp", IRAY_IMG_TYPE_BMP);
    gettimeofday(&start_tv, NULL);

    printf("time[%lu.%lu]\n", start_tv.tv_sec - end_tv.tv_sec, start_tv.tv_usec - end_tv.tv_usec);


    int x = 0;
    int y = 0;
    int pix = 0;
    sscanf(argv[1], "%d", &x);
    sscanf(argv[2], "%d", &y);
    sscanf(argv[3], "%d", &pix);

    FrameBufferDev fb_dev;
    
    ret = fb_dev.open("/dev/fb0");
    if (ret) {
        printf("open device fail\n");
        return ret;
    }

    fb_dev.showScreenInfo();

    fb_dev.prepareOutput();
    IrayRgbImage whiteImg;
//    whiteImg.create(1920, 1080, COLOR_TYPE_ARGB);
    fb_dev.outputImage(&rgbImage, x, y, 128);

    free(top_img);
    free(bottom_img);

    return 0;
}

int main(int argc, char *argv[])
{
    test_all(argc, argv) ;
    
    return 0;
}
