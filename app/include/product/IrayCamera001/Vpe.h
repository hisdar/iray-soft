#ifndef _VPE_H_
#define _VPE_H_

#include <common/IrayImage.h>

#define VPE_DEVICE_PATH "/dev/video0"

class Vpe {
public:
	Vpe();
	~Vpe();
	int formatSwap(IrayImage &src, IrayImage &dst);
	int open();

	int s_ctrl();

private:

	
	
	int m_dev;
};

#endif
