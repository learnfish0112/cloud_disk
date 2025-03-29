#include "head.h"

int clientMkdir(int sockfd, char *argu) {
    if(strcmp(argu, "\0") == 0) {
        printf("Arg error, please input your dir name\n");
        return CLIENTR_ERR;
    }

    Train train;
    bzero(&train, sizeof(Train));
    train.length = strlen(argu);
    strcpy(train.buf, argu);
    send(sockfd, &train, 4+train.length, 0);

    bzero(&train, sizeof(Train));
    recv(sockfd, &train.length, sizeof(int), MSG_WAITALL);
    recv(sockfd, train.buf, train.length, 0);
    if(strcmp(train.buf, "ok") == 0) {
        printf("user create dir status:%s\n", train.buf);
    } else {
        //can sperate user cause or server cause in future
        printf("user create dir failed\n");
    }

    return CLIENTR_OK;
}

