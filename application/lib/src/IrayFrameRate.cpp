#include <unistd.h>
#include <string.h>
#include <IrayFrameRate.h>

IrayFrameRate::IrayFrameRate(u32 timevalBufSize, u32 isPrint = 0)
{
	init(timevalBufSize, isPrint);
}

IrayFrameRate::IrayFrameRate()
{
	init(DEFAULT_BUFFER_SIZE, 0);
}

void IrayFrameRate::init(u32 timevalBufSize, u32 isPrint)
{
	m_isPrint = isPrint;
	m_startIndex = 0;
	m_endIndex = 0;
	m_timevalSize = 0;
	m_timevalBufSize = timevalBufSize;
	m_timevalArr = (struct timeval *)malloc(sizeof(struct timeval) * m_timevalBufSize);

	if (m_timevalArr == NULL) {
		iray_err("alloc mem fail\n");
	}
}

IrayFrameRate::~IrayFrameRate()
{
	delete[] m_timevalArr;
	m_timevalArr = NULL;
}

int IrayFrameRate::receiveFrame(IrayCameraData *frameData)
{
	int ret= 0;
	struct timeval tv;
	
	ret = gettimeofday(&tv, NULL);
	if (ret) {
		iray_err("get time of day fail, ret=%d\n", ret);
		return ret;
	}

	ret = pushTimeval(&tv);
	if (m_isPrint && m_timevalSize > 1) {
		iray_info("Frame rate:[%f/s]\r", getFrameRate());
	} else {
		iray_dbg("Do not print frame rate.\n");
	}

	return ret;
}

u32 IrayFrameRate::getTimevalSize()
{
	return m_timevalSize;
}

u32 IrayFrameRate::getTimevalBufSize()
{
	return m_timevalBufSize;
}

int IrayFrameRate::pushTimeval(struct timeval *timeval)
{
	if (m_timevalArr == NULL) {
		return -ENOMEM;
	}

	// buffer is full. drop head item
	if (m_endIndex == m_startIndex && m_timevalSize == m_timevalBufSize) {
		dropHeadTimeval();
	}

	memcpy(&m_timevalArr[m_endIndex], timeval, sizeof(struct timeval));

	if (m_endIndex == m_timevalBufSize - 1) {
		m_endIndex = 0;
	} else {
		m_endIndex += 1;
	}

	m_timevalSize += 1;

	return 0;
}

int IrayFrameRate::dropHeadTimeval()
{
	struct timeval tv;
	return popTimeval(&tv);
}

int IrayFrameRate::popTimeval(struct timeval *timeval)
{
	if (m_timevalSize <= 0) {
		return -ENODATA;
	}
	
	memcpy(timeval, &m_timevalArr[m_startIndex], sizeof(struct timeval));

	if (m_startIndex == m_timevalBufSize - 1) {
		m_startIndex = 0;
	} else {
		m_startIndex += 1;
	}

	m_timevalSize -= 1;

	return 0;
}

int IrayFrameRate::getHeadTimeval(struct timeval *timeval)
{
	if (m_timevalSize <= 0) {
		return -ENODATA;
	}

	memcpy(timeval, &m_timevalArr[m_startIndex], sizeof(struct timeval));
	
	return 0;
}

int IrayFrameRate::getTailTimeval(struct timeval *timeval)
{
	if (m_timevalSize <= 0) {
		return -ENODATA;
	}
	
	int lastIndex = 0;
	if (m_endIndex == 0) {
		lastIndex = m_timevalBufSize - 1;
	} else {
		lastIndex = m_endIndex - 1;
	}

	memcpy(timeval, &m_timevalArr[lastIndex], sizeof(struct timeval));
	
	return 0;
}

float IrayFrameRate::getFrameRate()
{
	int ret = 0;
	unsigned long diffUsec = 0;
	unsigned long frameUsec = 0;
	struct timeval stv, etv;

	if (m_timevalSize <= 0) {
		return 0;
	}

	ret = getHeadTimeval(&stv);
	if (ret) {
		return ret;
	}

	ret = getTailTimeval(&etv);
	if (ret) {
		return ret;
	}
	
	diffUsec = (etv.tv_sec - stv.tv_sec) * 1000000;
	diffUsec += etv.tv_usec - stv.tv_usec;
	frameUsec = diffUsec / (m_timevalSize - 1);

	return 1000000.0 / frameUsec;
}


