#include "head.h"


int ls(int sockfd, char *argus) {
    Train train;
    bzero(&train, sizeof(Train));

    train.length = strlen(argus);
    strcpy(train.buf, argus);
    send(sockfd, &train, sizeof(int)+train.length, 0);

    bool flag;
    recv(sockfd, &flag, sizeof(bool), 0);
    if(flag == false) {
        printf("User input dir error\n");
        return CLIENTR_ERR;
    }

    while(1) {
        int length;
        char dirName[USER_DIR_MAX_LEN] = {"\0"};
        recv(sockfd, &length, sizeof(int), 0);
        if(length == 0) {
            break;
        }
        recv(sockfd, dirName, length, 0);
        puts(dirName);
    }

    return CLIENTR_OK; 
}
