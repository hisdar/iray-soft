#ifndef _IRAY_THREAD_H_
#define _IRAY_THREAD_H_

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef ENOPROCESS
#define ENOPROCESS 1
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef u32
#define u32 unsigned int
#endif

#define SHM_KEY_BASE        0x53000001
#define SHM_KEY_MAX         0xFFFFFFFF
#define SHM_KEY_RETRY_STEP  0x00010000

#define SHM_MEM_STATE_READY     0x1001
#define SHM_MEM_STATE_HANDLED   0x1002

#define SHM_OFFSET_STAT     (0x00)
#define SHM_OFFSET_TYPE     (SHM_OFFSET_STAT + sizeof(int))
#define SHM_OFFSET_TIME_S   (SHM_OFFSET_TYPE + sizeof(int))
#define SHM_OFFSET_TIME_NS  (SHM_OFFSET_TIME_S + sizeof(long))
#define SHM_OFFSET_LEN      (SHM_OFFSET_TIME_NS + sizeof(long))
#define SHM_OFFSET_DATA     (SHM_OFFSET_TIME_NS + sizeof(int))

// this should less than 0xACFF(0xFFFF - 0x5300)
#define MAX_SHM_GET_RETRY   100

class IrayThread {
public:
    IrayThread();
    virtual ~IrayThread();

    int start();
    int send(u32 data_type, char *data, u32 len);
    virtual void stop() {};

    // this parameter is used for shm_key alloc
    static key_t g_shm_key;  // key_t is a int
protected:

    virtual void run() = 0;
    u32 getItemBufSize();
    int getEvent(char *buf, u32 *data_type, u32 len);

private:

    int getShmState();
    int getShmDataLen();
    int getShmDataType();
    int getShmTime(struct timeval * tv);
    int getShmData(char * buf, u32 len);

    int setShmState(int state);
    int setShmDataLen(int data_len);
    int setShmDataType(int data_type);
    int setShmTime(struct timeval * tv);
    int setShmData(char * data, u32 len);

    int charToInt(char * data);
    void intToChar(int value, char * out_buf);

    int initSharedMem();
    int releaseSharedMem();

    int m_shm_id;
    char *m_shm_addr;
    size_t m_shm_size;

    // this parameter is used in child process to get shared mem
    key_t m_shm_key;
    pid_t m_pid;

};

#endif
