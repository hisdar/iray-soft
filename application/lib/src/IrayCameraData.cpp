
#include <string.h>
#include <IrayCamera.h>
#include "IrayCameraData.h"

IrayCameraData::IrayCameraData()
{
	length = 0;
	addr = NULL;
}

IrayCameraData::IrayCameraData(char *address, int len)
{
	memset(&m_v4l2_fmt, 0x00, sizeof(struct v4l2_format));

	length = len;
	addr = address;
}

IrayCameraData::~IrayCameraData()
{

}

void IrayCameraData::setLength(int len)
{
	length = len;
}

void IrayCameraData::setAddr(char *address)
{
	addr = address;
}

int IrayCameraData::getLength()
{
	return length;
}

int IrayCameraData::getPixelByteCnt()
{
	return 2;
}

char *IrayCameraData::getAddr()
{
	return addr;
}

void IrayCameraData::setFormat(struct v4l2_format *fmt)
{
	if (fmt == NULL) {
		return;
	}

	memcpy(&m_v4l2_fmt, fmt, sizeof(struct v4l2_format));
}

void IrayCameraData::getFormat(struct v4l2_format *fmt)
{
	if (fmt == NULL) {
		return;
	}

	memcpy(fmt, &m_v4l2_fmt, sizeof(struct v4l2_format));
}

int IrayCameraData::getImageWidth()
{
	return m_v4l2_fmt.fmt.pix.width;
}

int IrayCameraData::getImageHeight()
{
	return m_v4l2_fmt.fmt.pix.height;
}


u32 IrayCameraData::getPixFmtType()
{
	return m_v4l2_fmt.fmt.pix.pixelformat;
}



