#ifndef __IRAY_CAMERA_RCV_H__
#define __IRAY_CAMERA_RCV_H__

#include <IrayCameraData.h>

class IrayCameraRcv {

public:
    IrayCameraRcv() {}
    ~IrayCameraRcv() {}
	
	virtual int receiveFrame(IrayCameraData *frameData) = 0;
};

#endif
