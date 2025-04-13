#include "head.h"

static void userGetFileName(char *argus, char *filename) {
    char* tempStr = NULL;
    char* savePtr = NULL;
    char* tempFile = NULL;

    tempStr = strtok_r(argus, "/", &savePtr);
    tempFile = tempStr;

    while((tempStr = strtok_r(NULL, "/", &savePtr)) != NULL) {
        //puts(tempStr);
        tempFile = tempStr;
    }

    puts(tempFile);
    strncpy(filename, tempFile, strlen(tempFile));
    return;
}

static int recvn(int sockfd, void *buf, int length){
    int total = 0;//already recv length
    char *p = (char *)buf;
    while(total < length){
        int ret = recv(sockfd,p+total,length-total,0);
        if(ret == 0){
            return CLIENTR_ERR;
        }
        total += ret;
    }
    return CLIENTR_OK;
}

int clientDownload(int sockfd, char *argus) {
    //Step1.send download filename
    //Step2. recv server response
    //       agree download->check file whether exist in local
    //              exist->return; else -> create file && recv
    //      not agree, note end-user,return
    //Step3. recv file need support breakpoint resume
    if(argus == NULL) {
        printf("please input your req download filename\n");
        return CLIENTR_INVALID_ARG;  
    }

    Train train;
    bzero(&train, sizeof(train));
    train.length = strlen(argus);
    strncpy(train.buf, argus, train.length);
    send(sockfd, &train, sizeof(int) + train.length, MSG_NOSIGNAL);

    //Step2. recv server response
    //       agree download->check file whether exist in local(now check by file size, obviously unreliable)
    //              exist->return; else -> create file && recv
    //      not agree, note end-user,return
    bzero(&train, sizeof(train));
    bool bAllowDownload = false;
    recv(sockfd, &bAllowDownload, sizeof(bool), 0);
    if(bAllowDownload == false) {
        printf("Your request is be reject, may be you don't have permission, you can check your request file exist or not to further confirm\n");
        return CLIENTR_ERR;
    }    

    char filename[USER_FILE_NAME_MAX] = {0};
    bzero(filename, sizeof(filename));
    userGetFileName(argus, filename);
    char filepath[USER_FILE_MAX_PATH_LENGTH] = {0};
    char *downloadpath = "./download/";
    strncat(filepath, downloadpath, strlen(downloadpath));
    strncat(filepath, filename, strlen(filename));
    printf("user download file path: %s\n", filepath);
    int fd = open(filepath, O_RDWR|O_CREAT, 0666);
    off_t filesize;
    int length = 0;
    recv(sockfd,&length,sizeof(int), MSG_WAITALL);
    recv(sockfd,&filesize,length, 0);

    struct stat statbuf;
    fstat(fd,&statbuf);
    printf("server filesize = %ld\n", filesize);
    printf("Local filesize： %ld\n", statbuf.st_size);
    bzero(&train, sizeof(train));
    train.length = sizeof(statbuf.st_size);
    memcpy(train.buf,&statbuf.st_size,train.length);
    send(sockfd,&train,sizeof(int)+train.length,MSG_NOSIGNAL);
    if(statbuf.st_size == filesize) {
        printf("Local already exist file\n");
        close(fd);
        return CLIENTR_OK;
    }

    //Step3. recv file need support breakpoint resume
    int curBufSize = 0;
    off_t fileRemanentSize = filesize - statbuf.st_size;
    if(filesize > USER_BIG_FILE_SIZE_MIN) {
        lseek(fd, statbuf.st_size, SEEK_SET);
        char buf[USER_RECV_FILE_BUF_SIZE] = {0};
        while(1) {
            if(fileRemanentSize != 0){
                printf("remanent length = %ld\n", fileRemanentSize);
            } else {
                //gets over
                printf("gets over!\n");
                break;
            }

            curBufSize = 0;
            recvn(sockfd, &curBufSize, sizeof(curBufSize));
            bzero(buf, sizeof(buf));
            recvn(sockfd, buf, curBufSize);
            write(fd, buf, curBufSize);
            fileRemanentSize -= curBufSize;
        }
    } else {
        ftruncate(fd,filesize);
        char *p = (char *)mmap(NULL,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        p = p + statbuf.st_size;
        recvn(sockfd,p,filesize);
        msync(p, filesize, MS_SYNC);
        munmap(p,filesize);
    } 

    close(fd);
    printf("Client receive %s success!\n", argus);
    return CLIENTR_OK;
}
