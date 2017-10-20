#ifndef _DIMENSIONS_H_
#define _DIMENSIONS_H_

#include <common/base-def.h>

class Dimensions {

public:
	Dimensions();
	Dimensions(u32 width, u32 height);
	~Dimensions();

	u32 getWidth();
	u32 getHeight();

	void setWidth(u32 width);
	void setHeight(u32 height);

	u32 width;
	u32 height;
};

#endif
