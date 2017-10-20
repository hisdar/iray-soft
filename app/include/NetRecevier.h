#ifndef __NET_RECEIVER_H__
#define __NET_RECEIVER_H__

#include <IrayCameraRcv.h>


class NetReceiver: public IrayCameraRcv {
public:
	NetReceiver();
	~NetReceiver();

	int receiveFrame(IrayCameraData *frameData); 
	int initSocket(char *ip_addr, int port);

	int bytesToInt(char* data);
	void intToBytes(int value, char *bytesData, int len);

private:
	int m_sock_fd;
	struct v4l2_format m_v4l2_fmt;

	int sendFrameSizeToSocket();
	int sendFrameDataToSocket(IrayCameraData *frameData);
};

#endif
