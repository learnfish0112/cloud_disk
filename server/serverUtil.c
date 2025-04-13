#include "head.h"

void serverCheckTildePosLegality(char *argus, bool *pbExistTildeExpectFirst) {
    int i = 1;
    int length = strlen(argus);

    for(; i < length; i++) {
        if(argus[i] == '~') {
            *pbExistTildeExpectFirst = true;
            break;
        }
    }

    return;
}

