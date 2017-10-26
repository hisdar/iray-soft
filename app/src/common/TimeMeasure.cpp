#include <string.h>
#include <common/base-def.h>
#include <common/TimeMeasure.h>

TimeMeasure::TimeMeasure(const char * name)
{
	strncpy(m_name, name, MAX_NAME_LEN - 1);
	gettimeofday(&m_oldTime, NULL);
}

TimeMeasure::~TimeMeasure()
{
	
}

void TimeMeasure::print()
{
	
	gettimeofday(&m_newTime, NULL);
	long sec = m_newTime.tv_sec - m_oldTime.tv_sec;
	long usec = m_newTime.tv_usec - m_oldTime.tv_usec;
	if (usec < 0) {
		usec += 1000000;
		sec -= 1;
	}

	iray_info("%s:time expend[%lu.%lu]\n", m_name, sec, usec);
}


