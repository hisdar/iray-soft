#ifndef __IRAY_CAMERA_DATA_H__
#define __IRAY_CAMERA_DATA_H__

#include <base-def.h>
#include <linux/videodev2.h>

class IrayCameraData {

public:
	IrayCameraData();
	IrayCameraData(char *addr, int len);
	~IrayCameraData();

	void setLength(int len);
	void setAddr(char *addr);

	int getLength();
	char *getAddr();

	void setFormat(struct v4l2_format *fmt);
	void getFormat(struct v4l2_format *fmt);
	int getImageWidth();
	int getImageHeight();

	u32 getPixFmtType();
	int getPixelByteCnt();

private:
	
	int length;
    char *addr;

	struct v4l2_format m_v4l2_fmt;
};

#endif
