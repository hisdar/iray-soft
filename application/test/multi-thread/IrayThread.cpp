
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <linux/stat.h>

#include "IrayThread.h"

int IrayThread::g_shm_key = SHM_KEY_BASE;

IrayThread::IrayThread()
{
    m_pid = 0;
    m_shm_id = 0;
    m_shm_key = 0;
    m_shm_addr = NULL;
    m_shm_size = getpagesize() * 512;  // 4k * 512 = 2M
}

IrayThread::~IrayThread()
{
    // TODO: shmdt()
    releaseSharedMem();
}

int IrayThread::releaseSharedMem()
{
    int ret = 0;
    pid_t pid = getpid();

    if (m_shm_addr == NULL) {
        printf("[%d]no shm addr to release\n", pid);
    } else {

        ret = shmdt(m_shm_addr);
        if (ret) {
            printf("[%d]release shm_addr fail:%s\n", pid, strerror(errno));
        } else {
            printf("[%d]release shm_addr success\n", pid);
        }

        struct shmid_ds ds;
        ret = shmctl(m_shm_id, IPC_STAT, &ds);
        if (!ret) {
            printf("[%d]attch:%lu\n", pid, ds.shm_nattch);
            ret = shmctl(m_shm_id, IPC_RMID, NULL);
        }
    }

    return 0;
}

int IrayThread::initSharedMem()
{
    for (int i = 0; i < MAX_SHM_GET_RETRY; i++) {

        m_shm_id = shmget(g_shm_key, m_shm_size, 0666 | IPC_CREAT | IPC_EXCL);
        if (m_shm_id == -1) {
            // get failed, retry
            g_shm_key += SHM_KEY_RETRY_STEP;
            continue;
        } else {

            m_shm_key = g_shm_key;
            g_shm_key += 1;
            shmctl(m_shm_id, SHM_LOCK, 0);
            printf("shm get success, key=0x%X, id=%d\n", m_shm_key, m_shm_id);
            return 0;
        }
    }

    printf("shmget fail\n");

    return -ENOMEM;
}

int IrayThread::start() {
    int ret = 0;

    ret = initSharedMem();
    if (ret) {
        return ret;
    }

    pid_t pid = fork();

    // create thread fail
    if (pid == -1) {
        return -ENOPROCESS;
    }

    // get shared mem addr
    m_shm_addr = (char *)shmat(m_shm_id, NULL, 0);
    if ((long)m_shm_addr == -1) {
        printf("shmat fail:%s\n", strerror(errno));
        return -ENOMEM;
    }

    // child thread
    if (pid == 0) {
        printf("child :shm_addr=%lu\n", (unsigned long)m_shm_addr);
        run();
        releaseSharedMem();
        exit(0);
    } else {
        sleep(1);
        // save child pid, in parent process;
        printf("parent:shm_addr=%lu\n", (unsigned long)m_shm_addr);
        m_pid = pid;
    }

    return SUCCESS;
}

int IrayThread::charToInt(char *data)
{
    int value = 0;
    
    value |= ((data[0] & 0xff) << 0);
    value |= ((data[1] & 0xff) << 8);
    value |= ((data[2] & 0xff) << 16);
    value |= ((data[3] & 0xff) << 24);

    return value;
}



void IrayThread::intToChar(int value, char *out_buf)
{
    out_buf[0] = (char)(value >> 0 & 0xff);
    out_buf[1] = (char)(value >> 8 & 0xff);
    out_buf[2] = (char)(value >> 16 & 0xff);
    out_buf[3] = (char)(value >> 24 & 0xff);
}

int IrayThread::getShmState()
{
    char *addr = (m_shm_addr + SHM_OFFSET_STAT);
    return *(int *)addr;
}

int IrayThread::setShmState(int state)
{
    char *addr = (m_shm_addr + SHM_OFFSET_STAT);
    // intToChar(state, m_shm_addr);
    *(int *)addr = state;
    return 0;
}

int IrayThread::getShmDataType()
{
    char *addr = (m_shm_addr + SHM_OFFSET_TYPE);
    return *(int *)addr;
}

int IrayThread::setShmDataType(int data_type)
{
    char *addr = (m_shm_addr + SHM_OFFSET_TYPE);
    *(int *)addr = data_type;
    return 0;
}

int IrayThread::getShmDataLen()
{
    char *addr = (m_shm_addr + SHM_OFFSET_LEN);
    return *(int *)addr;
}

int IrayThread::setShmDataLen(int data_len)
{
    char *addr = (m_shm_addr + SHM_OFFSET_LEN);
    *(int *)addr = data_len;
    return 0;
}

int IrayThread::getShmData(char *buf, u32 len)
{
    char *addr = m_shm_addr + SHM_OFFSET_DATA;
    memcpy(buf, addr, len);
    return 0;
}

int IrayThread::setShmData(char *data, u32 len)
{
    char *addr = m_shm_addr + SHM_OFFSET_DATA;
    memcpy(addr, data, len);
    return 0;
}

int IrayThread::setShmTime(struct timeval *tv)
{
    char *addr = m_shm_addr + SHM_OFFSET_TIME_S;
    *(long *)addr = tv->tv_sec;

    addr = m_shm_addr + SHM_OFFSET_TIME_NS;
    *(long *)addr = tv->tv_usec;

    return 0; 
}

int IrayThread::getShmTime(struct timeval *tv)
{
    char *addr = m_shm_addr + SHM_OFFSET_TIME_S;
    tv->tv_sec = *(long *)addr;
    
    addr = m_shm_addr + SHM_OFFSET_TIME_NS;
    tv->tv_usec = *(long *)addr;

    return 0;
}

u32 IrayThread::getItemBufSize()
{
    return m_shm_size;
}

/*  data format of shared mem
 *  --------------------------------------------------------
 *  |         |         |           |         |            |
 *  |  STATE  |  TYPE   |   TIME    |   LEN   |    DATA    |
 *  | [4bits] | [4bits] | [8bits-s] | [4bits] |    [LEN]   |
 *  |         |         | [8bits-u] |         |            |
 *  --------------------------------------------------------
 */
int IrayThread::send(u32 data_type, char *data, u32 len)
{
    u32 max_send_len = 0;
    struct timeval tv = {0};

    gettimeofday(&tv, NULL);

    max_send_len = m_shm_size - SHM_OFFSET_DATA;
    if (len > max_send_len) {
        printf("data is to long, max=%u, len=%u\n", max_send_len, len);
        return -ENODATA;
    }

    while (1) {
        int state = getShmState();
        if (state != SHM_MEM_STATE_READY) {
            setShmDataType(data_type);
            setShmTime(&tv);
            setShmDataLen(len);
            setShmData(data, len);
            setShmState(SHM_MEM_STATE_READY);
            break;
        } else {
            usleep(3);
        }
    }

    return 0;
}

int IrayThread::getEvent(char *buf, u32 *data_type, u32 len)
{
    u32 copy_len = 0;
    u32 shm_data_len = 0;
    struct timeval tv_data = {0};
    struct timeval tv_cur  = {0};

    while (TRUE) {
        int state = getShmState();
        if (state == SHM_MEM_STATE_READY) {

            *data_type = getShmDataType();
            getShmTime(&tv_data);

            shm_data_len = getShmDataLen();
            copy_len = shm_data_len;
            if (shm_data_len > len) {
                copy_len = len;
            }

            getShmData(buf, copy_len);

            setShmState(SHM_MEM_STATE_HANDLED);
            break;
        } else {
            usleep(5);
        }
    }

    gettimeofday(&tv_cur, NULL);
    long sec = tv_cur.tv_sec - tv_data.tv_sec;
    long usec = tv_cur.tv_usec - tv_data.tv_usec;
    if (usec < 0) {
        sec -= 1;
        usec += 1000000;
    }

    static long max_delay = 0;
    static long min_delay = 0xffffffff;
    static long max_speed = 0;
    static long min_speed = 0xffffffff;
    long speed = 2 * 1000000 / usec;

    max_speed = max_speed < speed ? speed : max_speed;
    min_speed = min_speed > speed ? speed : min_speed;

    max_delay = max_delay < usec ? usec : max_delay;
    min_delay = min_delay > usec ? usec : min_delay;

    if (*data_type == 2) {
        printf("time delay:[%u, %u], speed[%dM/s, %dM/s]\n",
            (unsigned int)min_delay, (unsigned int)max_delay,
            (unsigned int)min_speed, (unsigned int)max_speed);
    }
    return copy_len;
}

