#ifndef _LSB_STRUCT_INC
#define _LSB_STRUCT_INC
#define _LSB_NO_LUA
#include "main.h"
#undef _LSB_NO_LUA
enum EMessageID {
	ELSPMsgID_ProcessSnapshot = 0x20,
};
typedef struct {
	EMessageID msgID;
	void *data;
	int len;
} LSPBaseMsg;
typedef struct {
	int gameid;
	int profileid;
	char *data; //snapshot data \\gametype\\-666
	bool done;
} LSPSnapshotMsg;

typedef struct {
	modLoadOptions *options;
	MYSQL *conn;
} legacyStatsProcessor;
#endif
