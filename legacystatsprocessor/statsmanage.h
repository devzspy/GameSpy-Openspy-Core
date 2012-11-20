#include "main.h"
#include <legacystats/structs.h>
char *mergeKeys(char *keys,persisttype_t type,int index,int pid);
void getKeyString(int gameid, persisttype_t type, int index, int profileid, char *name, char *dst, int dstlen);
int getKeyInt(int gameid, persisttype_t type, int index, int profileid, char *name);
void setKeyInt(int gameid, persisttype_t type, int index, int profileid, char *name, int value);
void setKeyString(int gameid, persisttype_t type, int index, int profileid, char *name, char *value);
