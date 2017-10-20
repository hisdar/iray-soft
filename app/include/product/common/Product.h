#ifndef _PRODUCT_H_
#define _PRODUCT_H_

#include <IrayImage.h>

class Product {
public:
	Product();
	~Product();

	virtual int formatRawData(IrayImage &src, IrayImage &dst) = 0;
};
#endif
