#include "head.h"

int ls(int netfd, ThreadPool *pthreadPool, char *userName) {
    if(userName == NULL || pthreadPool == NULL) {
        return SERVER_RINVAL_ARG;
    }

    //find end-user
    int i;
    //&mutex 
    pthread_mutex_lock(&pthreadPool->taskQueue.mutex);
    for(i = 0; i < 100; ++i) {
        if(strcmp(pthreadPool->uesrArr[i].userName, userName) == 0) {
            break;
        }       
    }
    pthread_mutex_unlock(&pthreadPool->taskQueue.mutex);
    printf("user id = %d\n", i);

    //accept user input cmd args
    char path[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    int length;
    char argu[USER_CMD_ARGS_MAX_LEN] = {"\0"};
    recv(netfd, &length, sizeof(int), 0);
    printf("%d\n", length);
    if(length != 0) {
        recv(netfd, argu, length, 0);
    } 
    puts(argu);

    pthread_mutex_lock(&pthreadPool->taskQueue.mutex);
    getpath(path, argu, &pthreadPool->uesrArr[i].direcStack);
    pthread_mutex_unlock(&pthreadPool->taskQueue.mutex);
    puts(path);

    bool flag = true;
    DIR *pdir = opendir(path);
    NETDISK_LOG_DEBUG(pdir, NULL, "opendir");
    if(pdir == NULL) {
        flag = false;
        send(netfd, &flag, sizeof(bool), MSG_NOSIGNAL);
        return SERVER_RERR;
    }

    send(netfd, &flag, sizeof(bool), MSG_NOSIGNAL);
    struct dirent *pdirent;
    while(1) {
        pdirent = readdir(pdir);
        if(pdirent == NULL) {
            break;
        }
        int length = strlen(pdirent->d_name);
        send(netfd, &length, sizeof(int), MSG_NOSIGNAL);
        send(netfd, pdirent->d_name, length, MSG_NOSIGNAL);
    }
    //traverse dir end
    int exit = 0;
    send(netfd, &exit, sizeof(int), MSG_NOSIGNAL);

    closedir(pdir);
    return SERVER_ROK;
}
