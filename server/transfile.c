#include "head.h"
#include "util/util_head.h"


static void serverNoteUserDownloadFileRes(int netfd, bool flag) {
    bool bCmdExecRes = flag;
    send(netfd, &bCmdExecRes, sizeof(bool), MSG_NOSIGNAL);
}

int serverTransfile(int netfd, ThreadPool *threadpool, char *userName) {
    if(userName == NULL) {
        return SERVER_RINVAL_ARG; 
    }

    //Step1. find user idx in userArr
    //Step2. recv user want download filename,
    //       check arg legality first,  check file exist second,
    //      -> check ok, send filesize to client, recv client filesize for breakpoint resume(Specially,
    //       if client filesize == server filesize, regard as file already exist in client, return directly)
    //      -> check fail, not user res
    //Step3. transfile by client filesize 
    int i;                                                                                                                      
    for(i = 0; i < USER_MAX_COUNT; ++i) {
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        if(strcmp(threadpool->userArr[i].userName, userName) == 0) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            break;
        } else if(strcmp(threadpool->userArr[i].userName, userName) != 0 && i == USER_MAX_COUNT - 1) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            serverNoteUserDownloadFileRes(netfd, false);
            return SERVER_RERR_NOT_FOUND_USER;
        }
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    }

    //Step2. recv user want download filename,
    //       check arg legality first,  check file exist second,
    //      -> check ok, send filesize to client, recv client filesize for breakpoint resume(Specially,
    //       if client filesize == server filesize, regard as file already exist in client, return directly)
    //      -> check fail, note user gets false
    //
    //For client input path:
    //should support relative path, absolute path
    //case1. arg no "/", just filename, -> access file in current user path; else belong to other case
    //case2. arg first char is "~"-> change to home dir first, after that, handle remaining arg
    //Tips: for case2, don't allow ~ appear all position expect the first one.
    //case3. arg exist "." -> do nothing
    //case4. arg exist ".." -> pop dir stack
    //case5. relative path -> change user dir(need restore original dir)
    //most of the above case can handle by getpath API, I just need handle case2 Tips
    Train train;
    bzero(&train, sizeof(train));
    char clientInput[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    char filePath[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    recv(netfd, &train.length, sizeof(int), MSG_WAITALL);
    recv(netfd, clientInput, train.length, 0);

    bool bExistTildeExpectFirst = false;
    serverCheckTildePosLegality(clientInput, &bExistTildeExpectFirst);
    if(bExistTildeExpectFirst == true) {
        serverNoteUserDownloadFileRes(netfd, false);
        return SERVER_ROK;
    }

    bzero(&train, sizeof(train));
    pthread_mutex_lock(&threadpool->taskQueue.mutex);
    getpath(filePath, clientInput, &threadpool->userArr[i].direcStack);
    pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    puts(filePath);
    int ret = access(filePath, F_OK);
    if(ret != 0) {
        serverNoteUserDownloadFileRes(netfd, false);
        return SERVER_ROK;
    }
    serverNoteUserDownloadFileRes(netfd, true);

    int fd = open(filePath, O_RDWR);
    struct stat statbuf;
    fstat(fd,&statbuf);
    train.length = sizeof(statbuf.st_size);
    memcpy(train.data,&statbuf.st_size,train.length);
    send(netfd,&train,sizeof(int)+train.length,MSG_NOSIGNAL);
    printf("length: %ld\n", statbuf.st_size);
    off_t clientSize;
    int length;
    recv(netfd, &length, sizeof(int), 0);
    recv(netfd, &clientSize, length, 0);
    printf("clientSize = %ld\n", clientSize);
    if(clientSize == statbuf.st_size) {
        printf("file already exist\n");
        return SERVER_ROK;
    }

    //Step3. transfile by client filesize 
    //          need support breakpoint resume
    if(statbuf.st_size > USER_BIG_FILE_SIZE_MIN) {
        char *p = (char *)mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        printf("size = %ld\n",statbuf.st_size);
        while(clientSize < statbuf.st_size){
            if(statbuf.st_size - clientSize > TRANSFILE_BUF_MAX_SIZE){
                train.length = TRANSFILE_BUF_MAX_SIZE;
            }
            else{
                train.length = statbuf.st_size - clientSize;
            }
            send(netfd,&train.length,sizeof(int),MSG_NOSIGNAL);
            send(netfd,p+clientSize,train.length,MSG_NOSIGNAL);
            clientSize += train.length;
        }
        //train.length = 0;
        //send(netfd,&train.length,sizeof(int),MSG_NOSIGNAL);
        munmap(p,statbuf.st_size);
    } else {
        lseek(fd, clientSize, SEEK_SET);
        sendfile(netfd,fd,NULL,statbuf.st_size - clientSize);
    }

    close(fd);
 
    return SERVER_ROK;
}
