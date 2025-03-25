#include "head.h"

static void input_user_name(int sockfd, Train* ptrain) {
    char name[USER_NAME_MAX_LEN] = {"\0"};
    printf("Please input username: \n");
    //fgets(train.buf, sizeof(train.buf), stdin);
    read(STDIN_FILENO, name, USER_NAME_MAX_LEN);
    strncpy(ptrain->buf, name, strlen(name)-1);
    ptrain->length = strlen(ptrain->buf);
    //puts(ptrain->buf);
    send(sockfd, ptrain, sizeof(int)+ptrain->length, MSG_NOSIGNAL);
}

int clientLogin(int sockfd) {
    Train train;
    bzero(&train, sizeof(train));
    input_user_name(sockfd, &train);

    //accept verification information
    bool userExist = true;
    recv(sockfd, &userExist, sizeof(bool), 0);
    if(userExist == false) {
        printf("Username error!\n");
        return CLIENTR_ERR;
    }

    //accept server send salt
    char salt[TRAIN_CARRIAGE_MAX_LEN] = {0};
    recv(sockfd, &train.length, sizeof(int), MSG_WAITALL);
    recv(sockfd, &salt, train.length, 0);

    //将服务端收来的salt生成密文再发送(防止抓包)
    char* Password = getpass("Please input password：\n");
    strncpy(train.buf, Password, strlen(Password));

    char *encryptPassword = crypt(train.buf, salt);
    //puts(encryptPassword);

    //send cryptograph
    bzero(&train, sizeof(train));
    train.length = strlen(encryptPassword);
    strncpy(train.buf, encryptPassword, train.length);
    send(sockfd, &train, sizeof(int)+train.length, MSG_NOSIGNAL);

    bool checkPwd = true;
    recv(sockfd, &checkPwd, sizeof(int), 0);
    if(checkPwd == false) {
        printf("pwd input error!\n");
        return CLIENTR_ERR;
    }
    return CLIENTR_OK;
}

