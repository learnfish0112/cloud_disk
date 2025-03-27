#include "head.h"

int serverPwd(int netfd, ThreadPool *pthreadPool, char *userName) {
    if(userName == NULL || pthreadPool == NULL) {
        return SERVER_RINVAL_ARG;
    }

    //find end-user
    int i;
    //&mutex 
    pthread_mutex_lock(&pthreadPool->taskQueue.mutex);
    for(i = 0; i < USER_MAX_COUNT; ++i) {
        if(strcmp(pthreadPool->uesrArr[i].userName, userName) == 0) {
            break;
        }       
    }
    pthread_mutex_unlock(&pthreadPool->taskQueue.mutex);
    printf("user id = %d\n", i);


    char path[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    char argu[USER_CMD_ARGS_MAX_LEN] = {"\0"};
    int length = 0;
    pthread_mutex_lock(&pthreadPool->taskQueue.mutex);
    getpath(path, argu, &pthreadPool->uesrArr[i].direcStack);
    pthread_mutex_unlock(&pthreadPool->taskQueue.mutex);
    puts(path);
    length = strlen(path);

    send(netfd, &length, sizeof(int), MSG_NOSIGNAL);
    send(netfd, path, length, MSG_NOSIGNAL);

    return SERVER_ROK;
}
