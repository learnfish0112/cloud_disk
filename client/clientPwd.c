#include "head.h"


int clientPwd(int sockfd){
    Train temp_path;
    bzero(&temp_path, sizeof(temp_path));

    recv(sockfd, &temp_path.length, sizeof(int), MSG_WAITALL);
    recv(sockfd, temp_path.buf, temp_path.length, 0);
    puts(temp_path.buf);
    return CLIENTR_OK;
}
