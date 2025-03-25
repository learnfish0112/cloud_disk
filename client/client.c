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

    if(ret == CLIENTR_OK) {
        char sentence[USER_CMD_MAX_LEN] = {"\0"};
        printf("Please input your request: \n");
        read(STDIN_FILENO, sentence, sizeof(sentence));
        char *comd = strtok(sentence, " ");
        char comdArgus[USER_CMD_ARG_MAX_LEN];
        strcpy(comdArgus, sentence + (strlen(comd) + 1));//strcpy 2arg means str offset
        int argusize = strlen(comdArgus);//strlen will calculate '\n'
        comdArgus[argusize - 1] = '\0';//convert '\n' to '\0'

        Train train;
        train.length = strlen(comd);
        strcpy(train.buf, comd);
        send(sockfd, &train, 4+train.length, 0);

        if(strncmp(comd, "ls", 2) == 0) {
            printf("client input ls cmd\n");
            ls(sockfd, comdArgus);
        }
    }

    close(sockfd);
    return CLIENTR_OK;
}
