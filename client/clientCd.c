#include "head.h"


int clientCd(int sockfd, char *argus) {
    Train train;
    bzero(&train, sizeof(Train));

    train.length = strlen(argus);
    strcpy(train.buf, argus);
    send(sockfd, &train, sizeof(int)+train.length, 0);

    bool flag;
    recv(sockfd, &flag, sizeof(bool), 0);
    if(flag == false) {
        printf("User change dir failed!\n");
        return CLIENTR_ERR;
    } else {
        printf("User change dir success!\n");
        return CLIENTR_OK;
    }
}
