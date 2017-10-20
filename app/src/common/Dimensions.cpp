#include <common/Dimensions.h>

Dimensions::Dimensions()
{
	height = 0;
	width = 0;
}

Dimensions::Dimensions(u32 width, u32 height)
{
	this->height = height;
	this->width = width;
}

Dimensions::~Dimensions()
{

}

u32 Dimensions::getWidth()
{
	return width;
}

u32 Dimensions::getHeight()
{
	return height;
}

void Dimensions::setWidth(u32 width)
{
	this->width = width;
}

void Dimensions::setHeight(u32 height)
{
	this->height = height;
}


