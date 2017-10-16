#ifndef _IRAY_CLIENT_THREAD_H_
#define _IRAY_CLIENT_THREAD_H_

#include "IrayThread.h"

class IrayClientThread : public IrayThread {
public:
    IrayClientThread();
    ~IrayClientThread();

    virtual void run();
    virtual void stop();
};

#endif
