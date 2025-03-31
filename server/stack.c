#include "head.h"

int stackPush(Stack *pstack, const char *buf) {
    Node* newNode = (Node *)calloc(sizeof(Node), 1);
    NETDISK_CHECK_NULL(newNode);
    if(newNode == NULL) {
        return SERVER_RERR_ON_CLIB;
    }
    //record dir info
    strcpy(newNode->buf, buf);
    newNode->pnext = pstack->fhead;

    if(pstack->stackSize == 0) {
        pstack->ftail = newNode;
    }

    pstack->fhead = newNode;
    ++pstack->stackSize;
    
    return SERVER_ROK;
}

int stackPop(Stack *pstack) {
    if(pstack->stackSize == 0) {
        return SERVER_ROK;
    }

    Node* temp = pstack->fhead;
    pstack->fhead = pstack->fhead->pnext;
    free(temp);

    pstack->stackSize--;

    if(pstack->stackSize == 0) {
        printf("user in home dir now\n");
        pstack->fhead = pstack->ftail;
    }
    return SERVER_ROK;
}
