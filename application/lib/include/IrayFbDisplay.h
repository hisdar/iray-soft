#ifndef _IRAY_FRAME_RATE_H_
#define _IRAY_FRAME_RATE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <IrayCamera.h>
#include <IrayCameraRcv.h>

class IrayFbDisplay : public IrayCameraRcv {

public:
	IrayFbDisplay();
	~IrayFbDisplay();

	int receiveFrame(IrayCameraData *frameData);
};
#endif
