#include <common/gs_sesskey.h>

char *gs_sesskey(int sesskey) {
    int         i = 17;
    static char skbuff[9],
                *p;

    sprintf(skbuff, "%.8x", sesskey ^ 0x38f371e6);
    for(p = skbuff; *p; p++, i++) {
        *p += i;
    }
    return(skbuff);
}


