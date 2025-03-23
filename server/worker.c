#include "head.h"

int makeWorker(ThreadPool *threadpool) {
    for(int i = 0; i < threadpool->workerNum; ++i) {
        pthread_create(&threadpool->tidArr[i], NULL, threadFunc, threadpool);
    }

    return 0;
}

void *threadFunc(void *arg) {
    ThreadPool *threadpool = (ThreadPool *)arg;
    int i4_ret = SERVER_ROK;

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
        int netfd = threadpool->taskQueue.pfront->netfd;
        deQueue(&threadpool->taskQueue);
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);

        //start login
        char usrName[USER_NAME_MAX_LEN] = {"\0"};
        i4_ret = serverLogin(netfd, threadpool, usrName);
        if(i4_ret != SERVER_ROK)
        {
            printf("user login failed!\n");
            close(netfd);
            exit(0);
        }
        NETDISK_LOG_INFO(usrName, "login");

        //login success, start exec task
        int length = 0;
        char comd[CMD_MAX_LEN] = {"\0"};
        recv(netfd, &length, sizeof(int), 0);
        recv(netfd, comd, length, 0);
        NETDISK_LOG_INFO(usrName, comd);
    }

    return 0;
}
