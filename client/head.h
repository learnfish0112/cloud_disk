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
 
 #define USER_NAME_MAX_LEN 100
 #define TRAIN_CARRIAGE_MAX_LEN 2000
 
 typedef struct train_s{
     int length;
     char buf[TRAIN_CARRIAGE_MAX_LEN];
 }Train;
 
int clientLogin(int sockfd);
 #endif       
