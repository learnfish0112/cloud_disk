#include "head.h"

int initQueue(TaskQueue *ptaskQueue) {
    bzero(ptaskQueue, sizeof(TaskQueue));
    pthread_mutex_init(&ptaskQueue->mutex, NULL);
    pthread_cond_init(&ptaskQueue->cond, NULL);
    return SERVER_ROK;
}
