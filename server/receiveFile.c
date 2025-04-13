#include "head.h"
#include "util/util_head.h"

static int recvn(int sockfd, void *buf, int length){                                                                                                                  
    int total = 0;//already recv length
    char *p = (char *)buf;
    while(total < length){
        int ret = recv(sockfd,p+total,length-total,0);
        if(ret == 0){
            return SERVER_RERR;
        }
        total += ret;
    }
    return SERVER_ROK;
}

static void serverNoteUserReceiveFileStatus(int netfd, bool flag) {
    bool bCmdExecRes = flag;
    send(netfd, &bCmdExecRes, sizeof(bool), MSG_NOSIGNAL);
}

int serverReceiveFile(int netfd, ThreadPool *threadpool, char *userName) {
    //for server, just need know receive File path(include filename) && file size
    //Step1. find user idx in userArr
    //Step2. Receive client input file path,
    //       check path illegal ->if ok, exec step3, else note user fail
    //Step3. Receive client file size && open/create file to receive client send(need support breakpoint resume) data
    int i;                                                                                                                      
    for(i = 0; i < USER_MAX_COUNT; ++i) {
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        if(strcmp(threadpool->userArr[i].userName, userName) == 0) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            break;
        } else if(strcmp(threadpool->userArr[i].userName, userName) != 0 && i == USER_MAX_COUNT - 1) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            serverNoteUserReceiveFileStatus(netfd, false);
            return SERVER_RERR_NOT_FOUND_USER;
        }
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    }

    //Step2. Receive client input file path,
    //      check path legality->if ok, exec step3, else note user fail
    //      check legality action is same as transfile.c action
    Train train;
    bzero(&train, sizeof(train));
    char clientInput[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    char filePath[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    recv(netfd, &train.length, sizeof(int), MSG_WAITALL);
    recv(netfd, clientInput, train.length, 0);

    bool bExistTildeExpectFirst = false;
    serverCheckTildePosLegality(clientInput, &bExistTildeExpectFirst);
    if(bExistTildeExpectFirst == true) {
        printf("exist tilde expect first pos\n");
        serverNoteUserReceiveFileStatus(netfd, false);
        return SERVER_ROK;
    }
    serverNoteUserReceiveFileStatus(netfd, true);

    bzero(&train, sizeof(train));
    pthread_mutex_lock(&threadpool->taskQueue.mutex);
    getpath(filePath, clientInput, &threadpool->userArr[i].direcStack);
    pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    puts(filePath);

    //Step3. Receive client file size && open/create file to receive client send(need support breakpoint resume) data
    bzero(&train, sizeof(train));
    struct stat clientStatbuf;
    recv(netfd, &clientStatbuf, sizeof(clientStatbuf), 0);
    int fd = open(filePath, O_RDWR | O_CREAT);
    struct stat serverStatbuf;
    fstat(fd,&serverStatbuf);
    send(netfd, &serverStatbuf, sizeof(serverStatbuf), MSG_NOSIGNAL);
    printf("client upload filesize = %ld, server cur filesize = %ld\n", clientStatbuf.st_size, serverStatbuf.st_size);
    if(clientStatbuf.st_size == serverStatbuf.st_size) {
        return SERVER_ROK;
    }

    int curBufSize = 0;
    off_t clientUploadFileSize = clientStatbuf.st_size;
    off_t fileRemanentSize = clientUploadFileSize - serverStatbuf.st_size;
    if(clientUploadFileSize > USER_BIG_FILE_SIZE_MIN) {
        lseek(fd, serverStatbuf.st_size, SEEK_SET);
        char buf[SERVER_RECV_FILE_BUF_SIZE] = {0};
        while(1) {
            if(fileRemanentSize != 0){
                printf("remanent length = %ld\n", fileRemanentSize);
            } else {                                                                                                                                                  
                printf("server receive file over!\n");
                break;
            }

            curBufSize = 0;
            recvn(netfd, &curBufSize, sizeof(curBufSize));
            bzero(buf, sizeof(buf));
            recvn(netfd, buf, curBufSize);
            write(fd, buf, curBufSize);
            fileRemanentSize -= curBufSize;
        }
    } else {
        ftruncate(fd,clientUploadFileSize);
        char *p = (char *)mmap(NULL,clientUploadFileSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        p = p + serverStatbuf.st_size;
        recvn(netfd,p,(clientUploadFileSize - serverStatbuf.st_size));
        msync(p, clientUploadFileSize, MS_SYNC);
        munmap(p,clientUploadFileSize);
    } 

    close(fd);
    printf("Server recv %s success!\n", filePath);

    return SERVER_ROK;
}
