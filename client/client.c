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

    char sentence[USER_CMD_MAX_LEN] = {"\0"};
    char comdArgus[USER_CMD_ARG_MAX_LEN];
    while(ret == CLIENTR_OK) {
        bzero(sentence, sizeof(sentence));
        bzero(comdArgus, sizeof(comdArgus));
        printf("Please input your request: \n");
        read(STDIN_FILENO, sentence, sizeof(sentence));
        char *comd = strtok(sentence, " \n");//2arg means delim set,not strict matching
        strcpy(comdArgus, sentence + (strlen(comd) + 1));
        int argusize = strlen(comdArgus);//strlen will calculate '\n'
        comdArgus[argusize - 1] = '\0';//convert '\n' to '\0'

        Train train;
        train.length = strlen(comd);
        strcpy(train.buf, comd);
        send(sockfd, &train, 4+train.length, 0);

        //printf("Your input comd: %s, comd len = %ld\n", comd, strlen(comd));
        if(strcmp(comd, "ls\0") == 0) {
            printf("client input ls cmd\n");
            ls(sockfd, comdArgus);
        } else if(strcmp(comd, "pwd\0") == 0) {
            printf("client input pwd cmd\n");
            clientPwd(sockfd);
        } else if(strcmp(comd, "mkdir\0") == 0) {
            printf("client input mkdir cmd\n");
            clientMkdir(sockfd, comdArgus);
        } else if(strcmp(comd, "rmdir\0") == 0) {
            printf("client input rmdir cmd\n");
            clientRmdir(sockfd, comdArgus);
        } 
        else {
            printf("Unknown request, please reinput\n");
        }
    }

    close(sockfd);
    return CLIENTR_OK;
}
