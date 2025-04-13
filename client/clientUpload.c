#include "head.h"

int clientUpload(int sockfd, char *argus) {
    //Step1. Send client input file to server, accept server respond
    //      ->respond ok, step3; else, return
    //Step2. Send client upload file size to server && receive server current file size,(if server cur file size == upload file size, directly return)
    //      ->upload file by remeanent size
    if(argus == NULL) {
        //need add send server invaild arg to let server exit
        return CLIENTR_ERR; 
    }
    
    //Step1. Send client input file to server, accept server respond
    //      ->respond ok, step2; else, return
    Train train;
    bzero(&train, sizeof(train));
    train.length = strlen(argus);
    strncpy(train.buf, argus, train.length);
    send(sockfd, &train, sizeof(int) + train.length, MSG_NOSIGNAL);

    bool serverRespond = false;
    recv(sockfd, &serverRespond, sizeof(train.length),0);
    if(serverRespond == false){
        printf("client upload file may be not exist, please check\n");
        return CLIENTR_ERR;
    }

    //Step2. Send client upload file size to server && receive server current file size,(if server cur file size == upload file size, directly return)
    //      ->upload file by remeanent
    char filename[USER_FILE_NAME_MAX] = {0};
    bzero(filename, sizeof(filename));
    strncpy(filename, argus, strlen(argus));
    char filepath[USER_FILE_MAX_PATH_LENGTH] = {0};
    char *downloadpath = "./download/";
    strncat(filepath, downloadpath, strlen(downloadpath));
    strncat(filepath, filename, strlen(filename));
    puts(filepath);
    
    int fd = open(filepath, O_RDWR);
    struct stat statbuf;
    fstat(fd,&statbuf);
    send(sockfd, &statbuf, sizeof(statbuf), MSG_NOSIGNAL);
    struct stat serverStatbuf;
    recv(sockfd, &serverStatbuf, sizeof(serverStatbuf), 0);
    printf("sizeof(statbuf.st_size) = %ld\n", sizeof(statbuf.st_size));
    printf("Client Upload file size: %ld\n", statbuf.st_size);
    printf("serverCurFileSize= %ld\n", serverStatbuf.st_size);
    if(serverStatbuf.st_size == statbuf.st_size) {
        printf("file already exist, client no need upload!!!\n");
        return CLIENTR_OK;
    }

    bzero(&train, sizeof(train));
    off_t serverCurFileSize = serverStatbuf.st_size;
    if(statbuf.st_size > USER_BIG_FILE_SIZE_MIN) {
        char *p = (char *)mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        printf("size = %ld\n",statbuf.st_size);
        while(serverCurFileSize < statbuf.st_size){
            if(statbuf.st_size - serverCurFileSize > TRAIN_CARRIAGE_MAX_LEN){
                train.length = TRAIN_CARRIAGE_MAX_LEN;
            }
            else{
                train.length = statbuf.st_size - serverCurFileSize;
            }
            //support breakpoint resume
            send(sockfd,&train.length,sizeof(int),MSG_NOSIGNAL);
            send(sockfd,p+serverCurFileSize,train.length,MSG_NOSIGNAL);
            serverCurFileSize += train.length;
        }
        //train.length = 0;
        //send(sockfd,&train.length,sizeof(int),MSG_NOSIGNAL);
        munmap(p,statbuf.st_size);
    } else {
        //support breakpoint resume
        lseek(fd, serverCurFileSize, SEEK_SET);
        sendfile(sockfd,fd,NULL,statbuf.st_size - serverCurFileSize);
    }

    close(fd);
    printf("client upload:%s success!\n", argus);
    return CLIENTR_OK;
}
