#ifndef _VPE_CAPTURE_H_
#define _VPE_CAPTURE_H_

#include <common/base-def.h>
#include <Vpe.h>

class VpeCapture {
public:
	VpeCapture(int dev, Vpe *vpe);
	~VpeCapture();

private:
	int m_dev;
	Vpe *m_vpe;
	struct v4l2_format m_fmt;

	struct v4l2_buffer m_buffer;
};
#endif
