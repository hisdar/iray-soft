#ifndef _RAW_DATA_PARSER_H_
#define _RAW_DATA_PARSER_H_

#include <IrayCameraDataHandler.h>

class RawDataParser : public IrayCameraDataHandler {

public:
	RawDataParser();
	~RawDataParser();

	int handle(IrayCameraData *data);
};

#endif