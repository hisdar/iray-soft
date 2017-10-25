#ifndef _VPE_H_
#define _VPE_H_

#include <linux/videodev2.h>
#include <common/base-def.h>

#include <VpeOutput.h>
#include <VpeCapture.h>

#define VPE_DEVICE_PATH "/dev/video0"

class Vpe {
public:
	Vpe();
	~Vpe();

	int init(int srcWidth, int srcHeight, int srcFmt, int dstWidth, int dstHeight, int dstFmt);
	int put(void * data, u32 len);
	int get(void * data, u32 len);

private:
	int m_dev;
	VpeDev *m_vpeDev;
	VpeOutput *m_output;
	VpeCapture *m_capture;
};

#endif
