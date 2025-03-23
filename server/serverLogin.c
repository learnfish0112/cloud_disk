#include "head.h"

#define SALT_STR_LEN 1000
void get_salt(char* salt, char* password){
    int i, j;
    for(i = 0,j = 0; password[i] && j != 3;++i)
    {
        if(password[i] == '$')
            ++j;
    }
    strncpy(salt,password,i-1);
}

int serverLogin(int netfd, ThreadPool * threadpool, char * usrName){
    Train train;//msg format
    bzero(&train, sizeof(train));
    //record salt array
    char salt[SALT_STR_LEN] = {"\0"};
    char userName[USER_NAME_MAX_LEN] = {"\0"};
    int i4_ret = SERVER_ROK;

    //This  flag  requests  that the operation block until the full request is satisfied.
    recv(netfd, &train.length, sizeof(int), MSG_WAITALL);
    recv(netfd, &train.data, train.length, 0);
    printf("name=%s\n", train.data);
    strncpy(userName, train.data, train.length);

    //Get user info in shadow
    bool userExist = true; //check client user exist
    struct spwd *sp = getspnam(train.data);
    if(sp == NULL){
        //2 Possibility 1: user not exist 2: getspnam error
        //just easy hanlde, not add error handle now
        NETDISK_LOG_ERROR(sp, NULL);
        userExist = false;
        send(netfd, &userExist, sizeof(bool), MSG_NOSIGNAL);
        return SERVER_RERR;
    }
    send(netfd, &userExist, sizeof(bool), MSG_NOSIGNAL);

    //user exist, record
    //multi-thread, exist new cover old problem
    pthread_mutex_lock(&threadpool->taskQueue.mutex);
    strcpy(threadpool->uesrArr[threadpool->currIndex].userName, train.data);
    pthread_mutex_unlock(&threadpool->taskQueue.mutex);

    get_salt(salt, sp->sp_pwdp);
    printf("salt=%s\n", salt);
    //send salt val to client(actually, not only salt val)
    int length = strlen(salt);
    send(netfd, &length, sizeof(int), MSG_NOSIGNAL);
    send(netfd, salt, length, MSG_NOSIGNAL);
    //accept client cryptograph
    bzero(&train, sizeof(train));
    recv(netfd, &train.length, sizeof(int), MSG_WAITALL);
    recv(netfd, &train.data, train.length, 0);
    printf("client input passwd=%s\n", train.data);
    puts(sp->sp_pwdp); //Encrypted password

    //check user input pwd
    bool pwdCheckRes = false;
    if(strcmp(sp->sp_pwdp, train.data) == 0){
        printf("check success!\n");
        printf("Welcome, %s!\n", userName);
        pwdCheckRes = true;
    }
    else{
        send(netfd, &pwdCheckRes, sizeof(bool), MSG_NOSIGNAL);
        return SERVER_RERR;
    }
    send(netfd, &pwdCheckRes, sizeof(bool), MSG_NOSIGNAL);

    pthread_mutex_lock(&threadpool->taskQueue.mutex);
    strcpy(threadpool->uesrArr[threadpool->currIndex].encrypted, train.data);
    strcpy(usrName, threadpool->uesrArr[threadpool->currIndex].userName);//将正确的用户信息存入worker里面的usrName
    pthread_mutex_unlock(&threadpool->taskQueue.mutex);

    //check user dir exist
    if(access(threadpool->uesrArr[threadpool->currIndex].userName, F_OK) == 0){
        //exist
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        if(threadpool->uesrArr[threadpool->currIndex].direcStack.stackSize == 0) {
            //push dir into stack
            stackPush(&threadpool->uesrArr[threadpool->currIndex].direcStack, threadpool->uesrArr[threadpool->currIndex].userName);
        }
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);

        return 0;
    }else{
        //dir not exist, create user dir
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        umask(0000);
        i4_ret = mkdir(usrName, 0777);
        if(i4_ret == -1){
            NETDISK_LOG_ERROR(i4_ret, -1);
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            return SERVER_RERR;
        }
        stackPush(&threadpool->uesrArr[threadpool->currIndex].direcStack, threadpool->uesrArr[threadpool->currIndex].userName);
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    }

    //user num++
    pthread_mutex_lock(&threadpool->taskQueue.mutex);
    ++threadpool->currIndex;
    pthread_mutex_unlock(&threadpool->taskQueue.mutex);

    return SERVER_ROK;
}


