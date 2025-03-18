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

    return 0;
}
