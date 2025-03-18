#include "head.h"

int initThreadPool(ThreadPool *pthreadPool, int workerNum) {
    pthreadPool->tidArr = (pthread_t *)calloc(workerNum, sizeof(pthread_t));
    pthreadPool->workerNum = workerNum;
    initQueue(&pthreadPool->taskQueue);
    pthreadPool->exitFlag = 0;
    bzero(pthreadPool->uesrArr, sizeof(pthreadPool->uesrArr));
    pthreadPool->currIndex = 0;

    return 0;
}
