#include "head.h"

int makeWorker(ThreadPool *threadpool) {
    for(int i = 0; i < threadpool->workerNum; ++i) {
        pthread_create(&threadpool->tidArr[i], NULL, threadFunc, threadpool);
    }

    return 0;
}

void *threadFunc(void *arg) {
    ThreadPool *threadpool = (ThreadPool *)arg;

    while(1) {
        //Achieve task from taskQueue
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        while(threadpool->exitFlag == 0 && threadpool->taskQueue.queueSize == 0) {
            pthread_cond_wait(&threadpool->taskQueue.cond, &threadpool->taskQueue.mutex);
        }

        if(threadpool->exitFlag == 1) {
            //worker exit normally
            printf("child is going to exit\n");
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            pthread_exit(NULL);
        }

        printf("child get a task!\n");
        //int netfd = threadpool->taskQueue.pfront->netfd;
        deQueue(&threadpool->taskQueue);
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    }

    return 0;
}
