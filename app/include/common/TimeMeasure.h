#ifndef _TIME_MEASURE_H_
#define _TIME_MEASURE_H_

#include <sys/time.h>

class TimeMeasure {
public:
	TimeMeasure(const char *name);
	~TimeMeasure();
	void print();

private:
	char m_name[MAX_NAME_LEN];
	struct timeval m_oldTime;
	struct timeval m_newTime;
};

#endif
