#ifndef __HEAD__
#define __HEAD__
 
 #include <cloud_disk/func.h>
 
 /***********************************************
 *
 *           typedef, struct, macro
 *
 ************************************************/
 //Ret value
#define CLIENTR_OK 0
#define CLIENTR_ERR -1
#define CLIENTR_ERR_ON_CLIB -2
#define CLIENTR_INVALID_ARG -3
 
//Client common cfg
#define USER_NAME_MAX_LEN 100
#define TRAIN_CARRIAGE_MAX_LEN 2000
#define USER_CMD_MAX_LEN 1024
#define USER_CMD_ARG_MAX_LEN 512
#define USER_DIR_MAX_LEN 256
#define USER_FILE_NAME_MAX 128
#define USER_FILE_MAX_PATH_LENGTH (256 + 128)
#define USER_BIG_FILE_SIZE_MIN (100 * 1024 * 1024)
#define USER_RECV_FILE_BUF_SIZE (2048)
 
typedef struct train_s{
     int length;
     char buf[TRAIN_CARRIAGE_MAX_LEN];
 }Train;
 
int clientLogin(int sockfd);
int ls(int sockfd, char *argus);
int clientPwd(int sockfd);
int clientMkdir(int sockfd, char *argus);
int clientRmdir(int sockfd, char *argus);
int clientCd(int sockfd, char *argus);
int clientDownload(int sockfd, char *argus);
int clientUpload(int sockfd, char *argus);
#endif       
