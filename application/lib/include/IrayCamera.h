#ifndef __IRAY_CAMERA_H__
#define __IRAY_CAMERA_H__

#include <stdio.h>
#include <linux/videodev2.h>

#include <base-def.h>
#include <IrayCameraRcv.h>
#include <IrayCameraData.h>
#include <IrayCameraControler.h>

#define IRAY_DEBUG

#ifdef IRAY_DEBUG
#define iray_dbg(fmt, arg...)	\
        printf("%s%s-%d:" fmt , "[Iray][dbg]", __func__, __LINE__, ##arg)
#else
#define iray_dbg(fmt, arg...)
#endif

#define iray_err(fmt, arg...)	\
        printf("%s%s-%d:" fmt , "[Iray][err]", __func__, __LINE__, ##arg)
#define iray_info(fmt, arg...)	\
        printf("%s%s-%d:" fmt , "[Iray][inf]", __func__, __LINE__, ##arg)

#define CAMERA_PATH		"/dev/video1"

//#define IMAGE_WIDTH 	1024
//#define IMAGE_HEIGHT	768

#define IMAGE_WIDTH 	640
#define IMAGE_HEIGHT	480

//#define IMAGE_WIDTH 	720
//#define IMAGE_HEIGHT	576

#define DEFAULT_BUFFER_SIZE		10

typedef struct __v4l2_user_buffer {
    int index;
    int length;
    char *addr;
} v4l2_user_buffer;

class IrayCamera {

public:
	IrayCamera();
	~IrayCamera();

	IrayCameraData m_data;
	IrayCameraControler m_controler;

	virtual int open();
	virtual int close();

	virtual int setParameter();

	int startCapture(int count, u32 isPrintInfo = 0);

	void setFrameReceiver(IrayCameraRcv *frameReceiver);

private:
	int streamOff();
	int streamOn();
	int dQueBuf();
	int reqBufs();
	int queryBuf(int, v4l2_buffer*);
	int mmap(int, v4l2_buffer*);
	int mmaps();
	int qBuf(v4l2_buffer*);
	int qBufs();
	int setFormat(v4l2_format*);
	int setFormatDefault();
	int getFormat(v4l2_format*);
	int getFormatDefault();

	int printFrameInfo();
	
private:
	u32 m_isPrintFrameInfo;
	int m_cam_fd;

	IrayCameraRcv *frameReceiver;

	struct v4l2_format m_v4l2_fmt;
	struct v4l2_buffer m_v4l2_buf;
	struct v4l2_requestbuffers m_v4l2_req_bufs;
	v4l2_user_buffer m_user_buffer[DEFAULT_BUFFER_SIZE];
};


#endif
