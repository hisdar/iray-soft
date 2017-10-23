#ifndef _VPE_OUTPUT_H_
#define _VPE_OUTPUT_H_

#include <Vpe.h>
#include <common/base-def.h>

class VpeOutput {

public:
	VpeOutput(int dev, Vpe *vpe);
	~VpeOutput();

private:
	int m_dev;

	u32 m_num_planes;
	struct v4l2_plane *m_planes;

	Vpe *m_vpe;
	struct v4l2_format m_fmt;
	struct v4l2_selection m_selection;

	struct v4l2_buffer m_buffer;
	struct v4l2_requestbuffers m_req_buf;
};


#endif
