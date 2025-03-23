#include "head.h"

int main(int argc,char*argv[])
{
    ARGS_CHECK(argc, 2);

    char ip[128] = {"\0"};
    char port[10] = {"\0"};
    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL)
    {
        exit(1);
    }
    fscanf(fp, "%s", ip);
    fscanf(fp, "%s", port);
    fclose(fp);
    fp = NULL;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //sockaddr_in for user
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);

    //sockaddr for OS
    int ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    ERROR_CHECK(ret, -1, "connect");

    ret = clientLogin(sockfd);
    return CLIENTR_OK;
}
