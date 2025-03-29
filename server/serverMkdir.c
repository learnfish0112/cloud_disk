#include "head.h"

int serverMkdir(int netfd, ThreadPool * threadpool, char* userName){
    //find user idx
    int i;                                                                                                                      
    for(i = 0; i < USER_MAX_COUNT; ++i) {
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        if(strcmp(threadpool->uesrArr[i].userName, userName) == 0) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            break;
        } else if(strcmp(threadpool->uesrArr[i].userName, userName) != 0 && i == USER_MAX_COUNT - 1) {
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
    char path[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    recv(netfd, &train.length, sizeof(int), 0);
    recv(netfd, path, train.length, 0);

    char absolutePath[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};

    pthread_mutex_lock(&threadpool->taskQueue.mutex);
    getpath(absolutePath, path, &threadpool->uesrArr[i].direcStack);
    pthread_mutex_unlock(&threadpool->taskQueue.mutex);

    umask(0000);
    int ret = mkdir(absolutePath, 0777);
    NETDISK_LOG_DEBUG(ret, -1, "mkdir in serverMkdir");

    if(ret == 0) {
        Train train;
        bzero(&train, sizeof(train));
        char *msg = "ok";
        strncpy(train.data, msg, strlen(msg));
        train.length = strlen(train.data);
        send(netfd, &train, 4 + train.length, MSG_NOSIGNAL);
    }

    return SERVER_ROK;
}
