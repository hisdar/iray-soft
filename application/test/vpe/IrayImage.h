#ifndef _IRAY_IMAGE_H_
#define _IRAY_IMAGE_H_

#include <base-def.h>

class IrayImage {

public:
	IrayImage();
	~IrayImage();

private:
	u32 m_width;
	u32 m_height;
};

#endif
