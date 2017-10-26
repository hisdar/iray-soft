#ifndef _GRAPHICS_CARD_H_
#define _GRAPHICS_CARD_H_

#include "common.h"
#include "common-drm.h"
#include "common-modeset.h"
#include "common-drawing.h"
#include <linux/fb.h>
#include <IrayCameraRcv.h>
#include "IrayRgbImage.h"

#define GRAPHICS_CARD_PATH "/dev/dri/card0"

struct flip_data {
	uint32_t plane_id;
	struct framebuffer plane_buf;
	int w, h;
};

class GraphicsCard : public IrayCameraRcv {
public:
	GraphicsCard();
	~GraphicsCard();
	int init();
	void find_planes(int fd, struct modeset_out *modeset_list);
	virtual int receiveFrame(IrayCameraData *frameData);

private:
	int m_dev;
	IrayRgbImage m_src_img;
	struct modeset_out *m_modeset_list;
};

#endif
