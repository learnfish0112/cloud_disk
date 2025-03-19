#include "head.h"

int initQueue(TaskQueue *ptaskQueue) {
    bzero(ptaskQueue, sizeof(TaskQueue));
    pthread_mutex_init(&ptaskQueue->mutex, NULL);
    pthread_cond_init(&ptaskQueue->cond, NULL);
    return SERVER_ROK;
}

int enQueue(TaskQueue *ptaskQueue, int netfd) {
    if(ptaskQueue == NULL)
    {
        return SERVER_RERR;
    }
    Task *ptask = (Task *)calloc(1, sizeof(Task));
    NETDISK_CHECK_NULL(ptask);
    if(ptask == NULL)
    {
        return SERVER_RERR_ON_CLIB;
    }
    ptask->netfd = netfd;
    ptask->pnext = NULL;

    if(ptaskQueue->queueSize == 0)
    {
        ptaskQueue->pfront = ptask;
    } else {
        ptaskQueue->prear->pnext = ptask;
    }

    ptaskQueue->prear = ptask;
    ++ptaskQueue->queueSize;

    return SERVER_ROK;
}

int deQueue(TaskQueue *ptaskQueue) {
    NETDISK_CHECK_NULL(ptaskQueue);
    if(ptaskQueue == NULL)
    {
        return SERVER_RERR;
    }

    Task *pCurTask = ptaskQueue->pfront;
    ptaskQueue->pfront = pCurTask->pnext;
    free(pCurTask);
    pCurTask = NULL;
    --ptaskQueue->queueSize;

    if(ptaskQueue->queueSize == 0) {
        ptaskQueue->prear = NULL;
    }

    return SERVER_ROK;
}
