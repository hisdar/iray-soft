#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/ocl.hpp>

#ifndef TRUE
#define TRUE 1
#endif

void fill_img(cv::Mat &img, uchar val)
{
	int height = img.rows;
	int width  = img.cols;

	memset(img.data, val, height * width * 3);
}

int main(int argc, char *argv[])
{
	cv::VideoWriter vw;

	cv::ocl::setUseOpenCL(TRUE);

	// CV_FOURCC('D', 'I', 'V', 'X') = MPEG-4 codec
	//vw.open("my_video.mpg", CV_FOURCC('M','J','P','G'), 30.0, cv::Size( 640, 480 ), true);
	vw.open(argv[2], CV_FOURCC(argv[1][0],argv[1][1],argv[1][2],argv[1][3]),
		30.0, cv::Size( 640, 480 ), true);
	
	cv::Mat img(480, 640, CV_8UC3);
	for (int i = 0; i < 1200; i++) {
		fill_img(img, i);
		vw.write(img);
	}

	return 0;
}

