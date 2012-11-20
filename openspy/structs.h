#ifndef _STRUCT_INC
#define _STRUCT_INC
#include <list>
#include "config.h"
enum {
	LOGLEVEL_ERROR,
	LOGLEVEL_NOTICE,
	LOGLEVEL_INFO,
};
typedef struct _modInfo {
	char name[128];
	char description[128];
} modInfo;

typedef struct _keyData {
	char *name;
	uint8_t type;
} keyData;
typedef struct _gameInfo {
	int id;
	char *name;
	char *secretkey;
	uint16_t queryport;
	uint16_t backendflags;
	uint32_t servicesdisabled;
	keyData *pushKeys;
	uint8_t numPushKeys; //sb protocol sends as a byte so max of 255
} gameInfo;
typedef struct _modLoadOptions {
  uint32_t bindIP;
  char *mysql_server;
  char *mysql_user;
  char *mysql_password;
  char *mysql_database;
  gameInfo *games;
  int totalgames;
  bool (*sendMsgProc)(char *,char *, void *, int);
  void (*logMessageProc)(char *, int, char *, ...);
  gameInfo *(*gameInfoNameProc)(char *);
  gameInfo *(*gameInfoIDProc)(int);
  configVar *moduleArray;
  char * (*getConfStr)(configVar *, char *);
  int (*getConfInt)(configVar *, char *);
} modLoadOptions;
#endif
