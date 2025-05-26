#include "stringHandle.h"
#include "head.h"

void substring(char* dest,const char* src, int from, int size) {
    if(dest == NULL || src == NULL) {
        printf("%s para err!\n", __FUNCTION__);
        return;
    }

    strncpy(dest, src + from, size);
    return;
}
