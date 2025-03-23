#include "head.h"

int stackPush(Stack *pstack, const char *buf) {
    Node* newNode = (Node *)calloc(sizeof(Node), 1);
    NETDISK_CHECK_NULL(newNode);
    if(newNode == NULL) {
        return SERVER_RERR_ON_CLIB;
    }
    newNode->pnext = pstack->fhead;

    if(pstack->stackSize == 0) {
        pstack->ftail = newNode;
    }

    pstack->fhead = newNode;
    ++pstack->stackSize;
    
    return SERVER_ROK;
}

int stackPop(Stack *pstack) {
    return SERVER_ROK;
}
