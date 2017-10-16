#include "stdio.h"
#include <malloc.h>
#include "IrayThread.h"
#include "IrayClientThread.h"

int test()
{
    IrayThread *clientThread = NULL;
    //IrayThread *serverThread = NULL;

    clientThread = new IrayClientThread();
    clientThread->start();

    int size = 2048 * 1024 - SHM_OFFSET_DATA;
    char *buf = (char *)malloc(size);
    for (u32 i = 0; i < 10000; i++) {
        //printf("send message\n");
        clientThread->send(1, buf, (size));
        //sleep(1);
    }

    clientThread->send(2, buf, size);

    delete clientThread;
    clientThread = NULL;
    return 0;
}

int main(int argc, char *argv[])
{
    

    test();
    printf("main===\n");
    return 0;
}
