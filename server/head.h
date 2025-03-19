#ifndef __TASK_HEAD__
#define __TASK_HEAD__
#include <cloud_disk/func.h>
#include <shadow.h>

/***********************************************
*
*           typedef, struct, macro
*
************************************************/
//ret value
#define SERVER_ROK 0
#define SERVER_RERR -1
#define SERVER_RERR_ON_CLIB -2

//server default cfg
#define USER_MAX_COUNT 100
#define PENDING_CONNECT_MAX_NUM 10


#define NETDISK_CHECK_NULL(ret) {                                                                              \
    if(NULL == ret) {                                                                                          \
        syslog(LOG_ERR, "[ABORT] Find to be used pointer is NULL!!! line = %d file = %s func = %s\n",          \
               __LINE__, __FILE__, __FUNCTION__);                                                              \
    }                                                                                                          \
}

#define NETDISK_LOG_ERROR(retval, val) {                                  \
    if(retval != SERVER_ROK) {                                            \
        syslog(LOG_ERR, "[%s ERROR:%s] line = %d file = %s func = %s\n",  \
            strerror(errno), __LINE__, __FILE__, __FUNCTION__);           \
    }                                                                     \
}

#define NETDISK_LOG_DEBUG(retval, val, msg) {                             \
    if(retval == val) {                                                   \
       syslog(LOG_DEBUG, "[%s DEBUG:%s] line = %d file = %s func = %s\n",  \
            msg, strerror(errno), __LINE__, __FILE__, __FUNCTION__);      \
    }                                                                     \
}

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
    char buf[50];          //record file or dir name
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
    User uesrArr[USER_MAX_COUNT];     //record user info
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
#endif
