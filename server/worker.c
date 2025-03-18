#include "head.h"

int makeWorker(ThreadPool *threadpool) {
    for(int i = 0; i < threadpool->workerNum; ++i) {
        pthread_create(&threadpool->tidArr[i], NULL, threadFunc, threadpool);
    }

    return 0;
}

void *threadFunc(void *arg) {
    //ThreadPool *threadpool = (ThreadPool *)arg;

    return 0;
}
