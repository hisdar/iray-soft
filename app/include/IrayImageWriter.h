#ifndef _IRAY_IMAGE_WRITER_H_
#define _IRAY_IMAGE_WRITER_H_

#include <IrayCameraRcv.h>
#include <IrayCamera.h>

#define IRAY_IMAGE_FILE_TYPE_RAW	0x1
#define IRAY_IMAGE_FILE_TYPE_HEX	0X2

#define FILE_NAME_LEN				(128)
#define DEST_FILE_NAME_LEN			(FILE_NAME_LEN + 32)

class IrayImageWriter : public IrayCameraRcv {

public:
	IrayImageWriter(char *fileName, int fileType);
	~IrayImageWriter();

	int receiveFrame(IrayCameraData *frameData); 
	int saveToRawFile(IrayCameraData *frameData);
	int saveToHexFile(IrayCameraData *frameData);

private:
	int m_fileType;

	char m_fileName[FILE_NAME_LEN];
	u32  m_fileCnt;
}; 

#endif
