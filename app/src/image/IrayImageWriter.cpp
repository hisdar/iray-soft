#include <string.h>
#include <IrayImageWriter.h>

IrayImageWriter::IrayImageWriter(char *fileName, int fileType)
{
	m_fileCnt = 0;
	m_fileType = fileType;
	strncpy(m_fileName, fileName, FILE_NAME_LEN - 1);
}

IrayImageWriter::~IrayImageWriter()
{

}

int IrayImageWriter::receiveFrame(IrayCameraData *frameData)
{
	switch (m_fileType)
	{
		case IRAY_IMAGE_FILE_TYPE_RAW:
			return saveToRawFile(frameData);
		case IRAY_IMAGE_FILE_TYPE_HEX:
			return saveToHexFile(frameData);
		default:
			break;
	}
	
	return 0;
}

int IrayImageWriter::saveToRawFile(IrayCameraData *frameData)
{
	return 0;
}

int IrayImageWriter::saveToHexFile(IrayCameraData *frameData)
{
	int ret = 0;
	int index = 0;
	char destFileName[DEST_FILE_NAME_LEN] = {0};

	FILE *destFile = NULL;

	ret = snprintf(destFileName, DEST_FILE_NAME_LEN - 1, "%s-%d.ihex", m_fileName, m_fileCnt);
	if (ret <= 0) {
		return ret;
	}

	m_fileCnt += 1;

	destFile = fopen(destFileName, "w+");
	if (destFile == NULL) {
		iray_err("open file fail, file=%s\n", destFileName);
		return -ENOFOPN;
	}

	int imageWidth = frameData->getImageWidth();
	int imageHeight = frameData->getImageHeight();
	char *data = frameData->getAddr();
	for (int i = 0; i < imageHeight; i++) {
		for (int j = 0; j < imageWidth; j++) {
			index = ((i * imageWidth + j) * 2);
			fprintf(destFile, "%02x%02x ", data[index], data[index + 1]);
		}

		fprintf(destFile, "\n");
	}

	fflush(destFile);
	fclose(destFile);

	destFile = NULL;
	return 0;
}

