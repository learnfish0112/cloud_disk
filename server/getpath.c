#include "head.h"

int getpath(char *path, char *argu, Stack *pstack) {
    if(path == NULL || argu == NULL || pstack == NULL) {
        return SERVER_RINVAL_ARG;
    }

    //get user virtual dir path
    strncat(path, "./", 2);
    strncat(path, pstack->ftail->buf, strlen(pstack->ftail->buf));

    if(strcmp(argu, "\0") == 0) {
        //show user pcurrent dir
        Node *pcurr = pstack->fhead;
        //target dir:./a/b/c
        char pForConcatenate[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
        while(pcurr != pstack->ftail){
            char p[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
            strncat(p, "/", 1);
            strcat(p, pcurr->buf);
            strcat(p, pForConcatenate);
            strcpy(pForConcatenate, p);
            pcurr = pcurr->pnext;
        }
        strcat(path, pForConcatenate);
        return SERVER_ROK;
    } else {
        if(argu[0] == '~'){
            //show home dir
            //target dir:./a
            char *pHopeList = strtok(argu, "/");
            pHopeList = strtok(NULL, "/");
            strncat(path, "/", 1);
            while(pHopeList != NULL) {
                strncat(path, pHopeList, strlen(pHopeList));
                strncat(path, "/", 1);
                pHopeList = strtok(NULL, "/");
            }
        } else {
            //argu: ./
            Node *pcurr = pstack->fhead;
            char pForConcatenate[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
            while(pcurr != pstack->ftail){
                char p[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
                strncat(p, "/", 1);
                strcat(p, pcurr->buf);
                strcat(p, pForConcatenate);
                strcpy(pForConcatenate, p);
                pcurr = pcurr->pnext;
            }
            strcat(path, pForConcatenate);
            strncat(path, "/", 1);

            //not exec error handle, exist code security risks
            char *pHopeList = strtok(argu, "/");
            while(pHopeList != NULL) {
                strncat(path, pHopeList, strlen(pHopeList));
                strncat(path, "/", 1);
                pHopeList = strtok(NULL, "/");
            }
        }
        int length = strlen(path);
        path[length - 1] = '\0';
    }
    return SERVER_ROK;
}

