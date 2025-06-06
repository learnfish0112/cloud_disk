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
        char userName[USER_NAME_MAX_LEN] = {"\0"};
        i4_ret = serverLogin(netfd, threadpool, userName);
        if(i4_ret != SERVER_ROK)
        {
            //hope provide user 3 times mistake chance
            printf("user login failed!\n");
            close(netfd);
            exit(0);
        }
        NETDISK_ACTION_PRINT(userName, "login");

        //login success, start exec task
        int length = 0;
        char comd[USER_CMD_TYPE_MAX_LEN ] = {"\0"};

        while(i4_ret == SERVER_ROK) {
            length = 0;
            bzero(&comd[0], sizeof(comd));
            recv(netfd, &length, sizeof(int), 0);
            recv(netfd, comd, length, 0);
            NETDISK_ACTION_PRINT(userName, comd);

            //printf("user input cmd:%s\n", comd);
            if(strcmp(comd, "ls\0") == 0) { 
                printf("user input ls cmd\n");
                ls(netfd, threadpool, userName);
            } else if(strcmp(comd, "pwd\0") == 0) {
                printf("user input pwd cmd\n");
                serverPwd(netfd, threadpool, userName);
            } else if(strcmp(comd, "mkdir\0") == 0) {
                printf("user input mkdir cmd\n");
                serverMkdir(netfd, threadpool, userName);
            } else if(strcmp(comd, "rmdir\0") == 0) {
                printf("user input rmdir cmd\n");
                serverRmdir(netfd, threadpool, userName);
            } else if(strcmp(comd, "cd\0") == 0) {
                printf("user input cd cmd\n");
                serverCd(netfd, threadpool, userName);
            } else if(strcmp(comd, "gets\0") == 0) {
                printf("user input gets cmd\n");
                serverTransfile(netfd, threadpool, userName);
            } else if(strcmp(comd, "push\0") == 0) {
                printf("user input push cmd\n");
                serverReceiveFile(netfd, threadpool, userName);
            } else {
                break;
            }
        }
    }

    return 0;
}
