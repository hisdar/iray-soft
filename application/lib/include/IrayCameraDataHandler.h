#ifndef _IRAY_CAMERA_DATA_HANDLER_H_
#define _IRAY_CAMERA_DATA_HANDLER_H_

class IrayCameraDataHandler {

	virtual int handle(IrayCameraData *data) = 0;
};

#endif