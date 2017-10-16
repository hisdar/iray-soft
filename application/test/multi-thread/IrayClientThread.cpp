#include <stdlib.h>
#include <stdio.h>
#include "IrayClientThread.h"

IrayClientThread::IrayClientThread()
{

}

IrayClientThread::~IrayClientThread()
{
    printf("~IrayClientThread\n");
}

void IrayClientThread::stop()
{

}

void IrayClientThread::run()
{
    u32 buf_size = 0;
    u32 data_len = 0;
    u32 data_type = 0;

    char *buf = NULL;

    buf_size = getItemBufSize();
    buf = (char *)malloc(buf_size);

    //printf("start to get event\n");
    while (TRUE) {
        data_len = getEvent(buf, &data_type, buf_size);
        if (data_type == 2) {
            printf("child end!!!\n");
        }
    }
}

