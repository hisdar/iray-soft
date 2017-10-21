#ifndef __IRAY_CAMERA_H__
#define __IRAY_CAMERA_H__

#include <stdio.h>
#include <linux/videodev2.h>

#include <common/base-def.h>
#include <IrayCameraRcv.h>
#include <IrayCameraData.h>
#include <IrayCameraControler.h>

#define CAMERA_PATH		"/dev/video1"

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

	void setFieldType(u32 fieldType);

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
	int notifyFrameData(v4l2_user_buffer *frameData);
	int initFrameBuffer(v4l2_user_buffer *srcData);

private:
	u32 m_is_print_frame_info;
	u32 m_field_type;
	int m_cam_fd;
		
	IrayCameraRcv *m_frame_receiver;
	IrayCameraData m_frame_data;

	struct v4l2_format m_v4l2_fmt;
	struct v4l2_buffer m_v4l2_buf;
	struct v4l2_requestbuffers m_v4l2_req_bufs;
	v4l2_user_buffer m_user_buffer[DEFAULT_BUFFER_SIZE];

	v4l2_user_buffer m_frame_buf;
};


#endif
