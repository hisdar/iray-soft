#ifndef _IRAY_FRAME_RATE_H_
#define _IRAY_FRAME_RATE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <IrayCamera.h>
#include <IrayCameraRcv.h>

#define DEFAULT_TIME_BUF	32

class IrayFrameRate : public IrayCameraRcv {

public:
	IrayFrameRate();
	IrayFrameRate(u32 timevalBufSize, u32 isPrint);
	~IrayFrameRate();

	int receiveFrame(IrayCameraData *frameData);
	float getFrameRate();

private:

	u32 m_timevalBufSize;
	u32 m_timevalSize;
	u32 m_startIndex;
	u32 m_endIndex;
	u32 m_isPrint;
	
	struct timeval *m_timevalArr;

	void init(u32 timevalBufSize, u32 isPrint);
	u32 getTimevalSize();
	u32 getTimevalBufSize();
	int dropHeadTimeval();
	int pushTimeval(struct timeval *timeval);
	int popTimeval(struct timeval *timeval);
	int getTailTimeval(struct timeval *timeval);
	int getHeadTimeval(struct timeval *timeval);
};

#endif
