#include "head.h"

//Define
#define EPOLL_WAIT_BLOCK_INDEFINITE -1

int exitPipe[2];

void handler(int signum) {
    printf("sig %d is comming!\n", signum);
    write(exitPipe[1], "1", 1);
}

int main(int argc,char*argv[])
{
    //./server cfg_file_name
    ARGS_CHECK(argc, 2);
    
    pipe(exitPipe);
    if(fork()) {
        //1.Parent process
        //On  success,  the PID of the child process is returned in the parent, and 0 is returned in the child.
        //pipefd[0] refers to the read end of the pipe.  pipefd[1] refers to the write end of the pipe.
        close(exitPipe[0]);
        signal(SIGUSR1, handler);
        wait(NULL);
        exit(0);
    } //parent ps cannot exist if, after accept signal usr1?
 
    //2.Child process
    //child ps can exec by the thread pool
    close(exitPipe[1]);

    //get IP，port，workerNum
    char ip[128] = {"\0"};
    char port[10] = {"\0"};
    char strWorkerNum[10] = {"\0"};
    FILE *fp = fopen(argv[1], "r");
    NETDISK_CHECK_NULL(fp);
    if(fp == NULL)
    {
        exit(1);
    }
    fscanf(fp, "%s", ip);
    fscanf(fp, "%s", port);
    fscanf(fp, "%s", strWorkerNum);
    fclose(fp);
    fp = NULL;
    int workerNum = atoi(strWorkerNum);

    //init thread pool by workerNum
    ThreadPool threadPool;
    initThreadPool(&threadPool, workerNum);
    makeWorker(&threadPool);

    int sockfd;
    tcpInit(ip, port, &sockfd);
    //accept arg just need > 1, no actual effect
    int epfd = epoll_create(1);
    epollAdd(epfd, sockfd);
    epollAdd(epfd, exitPipe[0]);

    //IO multiplexing
    struct epoll_event readySet[10];
    int readyNum = 0;
    int i4_ret = SERVER_ROK;
    while(1) {
        //a timeout of -1 causes epoll_wait() to block indefinitely, 0 cause return immediately
        readyNum = 0;
        printf("start wait client connect!\n");
        readyNum = epoll_wait(epfd, readySet, SIZE(readySet), EPOLL_WAIT_BLOCK_INDEFINITE);
        NETDISK_LOG_DEBUG(readyNum, -1, "epoll_wait");

        for(int i = 0; i < readyNum; ++i) {
            //new connect coming
            if(readySet[i].data.fd == sockfd) {
                int netfd = accept(sockfd, NULL, NULL);
                NETDISK_LOG_DEBUG(netfd, -1, "accept");
                pthread_mutex_lock(&threadPool.taskQueue.mutex);
                i4_ret = enQueue(&threadPool.taskQueue, netfd);
                NETDISK_LOG_DEBUG(i4_ret, SERVER_RERR_ON_CLIB, "enQueue");
                printf("main send a new task!\n");
                pthread_cond_broadcast(&threadPool.taskQueue.cond);
                //let worker thread to contend the mutex
                pthread_mutex_unlock(&threadPool.taskQueue.mutex);
            }
        }
    }

    return 0;
}
