#include "head.h"
#include "util/util_head.h"

int serverHandleUserRelativePath(int netfd, ThreadPool *threadpool, char *userName, char *path, char *absolutePath) {
    //find user idx
    int i;                                                                                                                      
    for(i = 0; i < USER_MAX_COUNT; ++i) {
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        if(strcmp(threadpool->userArr[i].userName, userName) == 0) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            break;
        } else if(strcmp(threadpool->userArr[i].userName, userName) != 0 && i == USER_MAX_COUNT - 1) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            Train train;
            bzero(&train, sizeof(train));
            char *error_msg = "user not found";
            strncpy(train.data, error_msg, strlen(error_msg));
            train.length = strlen(train.data);
            send(netfd, &train, 4 + train.length, MSG_NOSIGNAL);
            return SERVER_RERR_NOT_FOUND_USER;
        }
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    }

    //accept client relative path
    Train train;
    recv(netfd, &train.length, sizeof(int), 0);
    recv(netfd, path, train.length, 0);


    pthread_mutex_lock(&threadpool->taskQueue.mutex);
    getpath(absolutePath, path, &threadpool->userArr[i].direcStack);
    pthread_mutex_unlock(&threadpool->taskQueue.mutex);

    return SERVER_ROK;
}

int serverMkdir(int netfd, ThreadPool * threadpool, char* userName){
    char path[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    char absolutePath[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    int ret = SERVER_ROK; 
    
    ret = serverHandleUserRelativePath(netfd, threadpool, userName, path, absolutePath); 
    if(ret != SERVER_ROK) {
        return SERVER_RERR;
    }

    umask(0000);
    ret = mkdir(absolutePath, 0777);
    NETDISK_LOG_DEBUG(ret, -1, "mkdir in serverMkdir");

    serverCheckCmdExecStatus(netfd, ret);

    return SERVER_ROK;
}

int serverRmdir(int netfd, ThreadPool *threadpool, char *userName) {
    char path[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    char absolutePath[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    int ret = SERVER_ROK;

    serverHandleUserRelativePath(netfd, threadpool, userName, path, absolutePath); 
    if(ret != SERVER_ROK) {
        return SERVER_RERR;
    }

    ret = rmdir(absolutePath);

    serverCheckCmdExecStatus(netfd, ret);

    return SERVER_ROK;
}

