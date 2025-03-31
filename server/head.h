#ifndef __TASK_HEAD__
#define __TASK_HEAD__
#include <cloud_disk/func.h>
#include <shadow.h>

/***********************************************
*
*           typedef, struct, macro
*
************************************************/
//Ret value
#define SERVER_ROK 0
#define SERVER_RERR -1
#define SERVER_RERR_ON_CLIB -2
#define SERVER_RINVAL_ARG -3
#define SERVER_RERR_NOT_FOUND_USER -4

//Server common config
#define USER_NAME_MAX_LEN 100
#define USER_MAX_COUNT 100
#define PENDING_CONNECT_MAX_NUM 10
#define SMALL_TRAIN_SIZE 2000
#define ENCRYPTED_PWD_MAX_LEN 256
#define USER_CMD_TYPE_MAX_LEN 10
#define USER_VIRTUAL_DIR_PATH_MAX_LEN 1000
#define USER_CMD_ARGS_MAX_LEN 1000

#define NETDISK_CHECK_NULL(ret) {                                                                              \
    if(NULL == ret) {                                                                                          \
        syslog(LOG_ERR, "[ABORT] Find to be used pointer is NULL!!! line = %d file = %s func = %s\n",          \
               __LINE__, __FILE__, __FUNCTION__);                                                              \
    }                                                                                                          \
}

#define NETDISK_LOG_ERROR(retval, val) {                                  \
    if(retval == val) {                                            \
        syslog(LOG_ERR, "[ERROR:%s] line = %d file = %s func = %s\n",  \
            strerror(errno), __LINE__, __FILE__, __FUNCTION__);           \
    }                                                                     \
}

#define NETDISK_LOG_DEBUG(retval, val, msg) {                             \
    if(retval == val) {                                                   \
       syslog(LOG_DEBUG, "[%s DEBUG:%s] line = %d file = %s func = %s\n",  \
            msg, strerror(errno), __LINE__, __FILE__, __FUNCTION__);      \
    }                                                                     \
}

#define NETDISK_ACTION_PRINT(useName, action) {                         \
    syslog(LOG_INFO, "clinet username: %s Action: %s\n",useName, action); \
}

#ifdef __cplusplus
extern "C" {
#endif

    //may be need used

#ifdef __cplusplus
}
#endif

//small train
typedef struct train_s{
    int length;            //train head, record carriage length
    char data[SMALL_TRAIN_SIZE];       //carriage
}Train;

//task node
typedef struct task_s{
    int netfd;
    struct task_s *pnext;
}Task;

//taskQueue
typedef struct taskQueue_s{
    Task *pfront;          //queue head
    Task *prear;           //queue tail
    int queueSize;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}TaskQueue;

//dir Stack node
typedef struct node_s{
    char buf[50];          //record dir name
    struct node_s *pnext;
}Node;

//dir Stack
typedef struct stack_s{
    Node *fhead;
    Node *ftail;
    int stackSize;
}Stack;

//user info
typedef struct user_s{
    char userName[50];
    char encrypted[256];   //cryptograph
    Stack direcStack;      //save virtual dir stack
}User;

//threadPool
typedef struct threadPool_s{
    pthread_t *tidArr;     //record thread id
    int workerNum;         //work thread number
    TaskQueue taskQueue;   
    int exitFlag;          
    User userArr[USER_MAX_COUNT];     //record user info
    int currIndex;         
}ThreadPool;

//API
int initQueue(TaskQueue *ptaskQueue);
int enQueue(TaskQueue *ptaskQueue, int netfd);
int deQueue(TaskQueue *ptaskQueue);
int initThreadPool(ThreadPool *pthreadPool, int workerNum);
int makeWorker(ThreadPool *pthreadPool);
void *threadFunc(void *arg);
int tcpInit(const char *ip, const char *port, int *psocket);
int epollAdd(int epfd, int fd);
int serverLogin(int netfd, ThreadPool * threadpool, char * userName);
int stackPush(Stack *pstack, const char *buf);
int stackPop(Stack *pstack);
int ls(int netfd, ThreadPool *pthreadPool, char *userName);
int serverPwd(int netfd, ThreadPool *pthreadPool, char *userName);
int serverMkdir(int netfd, ThreadPool *pthreadPool, char *userName);
int serverRmdir(int netfd, ThreadPool *pthreadPool, char *userName);
int serverCheckCmdExecStatus(int netfd, int ret);
int serverCd(int netfd, ThreadPool *pthreadpool, char *userName);
#endif
