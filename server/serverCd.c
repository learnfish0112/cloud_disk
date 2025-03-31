#include "head.h"

static int serverCheckUserDirLegal(char *userPath) {
    int i = 0;
    int length = strlen(userPath);

    for(; i < length; i++) {
        if(userPath[i] == '~' && i != 0) {
            return SERVER_RINVAL_ARG;
        }
    }

    return SERVER_ROK;
}

static void serverNoteUserChangeDirRes(int netfd, bool flag) {
    bool bCmdExecRes = flag;
    send(netfd, &bCmdExecRes, sizeof(bool), MSG_NOSIGNAL);
}

static int serverChangeUserDirToHome(ThreadPool *threadpool, int userIdx) {
    Node* fCur = threadpool->userArr[userIdx].direcStack.fhead;
    while(fCur != threadpool->userArr[userIdx].direcStack.ftail) {
        stackPop(&threadpool->userArr[userIdx].direcStack);
        fCur = threadpool->userArr[userIdx].direcStack.fhead;
    }

    return SERVER_ROK;
}

int serverCd(int netfd, ThreadPool *threadpool, char *userName) {
    int i4Ret = SERVER_ROK;

    //find user idx
    int i;                                                                                                                      
    for(i = 0; i < USER_MAX_COUNT; ++i) {
        pthread_mutex_lock(&threadpool->taskQueue.mutex);
        if(strcmp(threadpool->userArr[i].userName, userName) == 0) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            break;
        } else if(strcmp(threadpool->userArr[i].userName, userName) != 0 && i == USER_MAX_COUNT - 1) {
            pthread_mutex_unlock(&threadpool->taskQueue.mutex);
            serverNoteUserChangeDirRes(netfd, false);
            return SERVER_RERR_NOT_FOUND_USER;
        }
        pthread_mutex_unlock(&threadpool->taskQueue.mutex);
    }

    //recv client arg
    Train train;
    char path[USER_VIRTUAL_DIR_PATH_MAX_LEN] = {"\0"};
    recv(netfd, &train.length, sizeof(int), 0);
    recv(netfd, path, train.length, 0);

    //check arg legally
    i4Ret = serverCheckUserDirLegal(path); 
    if(i4Ret != SERVER_ROK) {
        serverNoteUserChangeDirRes(netfd, false);
        return SERVER_RINVAL_ARG;
    }

    //In our clouddisk, only exist ~, not exist //
    //case1. no arg->change dir to home
    //case2. arg first char is "~"
    //case3. "."
    //case4. ".."
    //case5. "relative path"
    if(strlen(path) == 0) {
        //case1. pop user dir stack
        printf("case1. no arg\n");
        serverChangeUserDirToHome(threadpool, i);
    } else {
        //case2. arg first char is "~"
        if(path[0] == '~') {
            printf("case2. ~\n");
            serverChangeUserDirToHome(threadpool, i);
        } else {
           char *temp_path; 
           temp_path = strtok(path,"/");
           while(temp_path != NULL) {
               if(strlen(temp_path) == 0 || \
                  (strlen(temp_path) == 1 && temp_path[0] == '.')) {
                    //case3. "."
                    //do nothing
                    printf("case3. one point -> do nothing\n");
               } else if(strcmp(temp_path, "..") == 0) {
                    //case4. ".."
                    //pop Stack, if user in home dir, do nothing
                    printf("case4. two point -> pop stack dir\n");
                    stackPop(&threadpool->userArr[i].direcStack);
               } else {
                    //case5. "relative path"
                    //get current path-> concatenate(push)
                    printf("case5. relative path  -> push stack dir\n");
                    stackPush(&threadpool->userArr[i].direcStack, temp_path);
               }
                
               temp_path = strtok(NULL,"/");
           }
        }
    }
    
    serverNoteUserChangeDirRes(netfd, true);
    return SERVER_ROK;
}
