#include "head.h"

//check status && send result to user
int serverCheckCmdExecStatus(int netfd, int ret) {
    if(ret == 0) {
        Train train;
        bzero(&train, sizeof(train));
        char *msg = "ok";
        strncpy(train.data, msg, strlen(msg));
        train.length = strlen(train.data);
        send(netfd, &train, 4 + train.length, MSG_NOSIGNAL);
    } else {
        Train train;
        bzero(&train, sizeof(train));
        char *msg = "failed";
        strncpy(train.data, msg, strlen(msg));
        train.length = strlen(train.data);
        send(netfd, &train, 4 + train.length, MSG_NOSIGNAL);
    }

    return SERVER_ROK;
}

