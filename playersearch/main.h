#ifndef _MAIN_PLAYERSEARCH_INC
#define _MAIN_PLAYERSEARCH_INC
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#define close closesocket
#define strcasecmp stricmp
typedef int socklen_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/times.h>
#include <stropts.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#define stricmp strcasecmp
#define sprintf_s snprintf
#define strnicmp strncasecmp
#define SEARCHPORT 29901
#include <openspy/structs.h>
#include "lookup.h"
extern MYSQL *conn;
extern MYSQL_RES *res;
extern MYSQL_ROW row;
typedef struct {
	int sd;
}threadOptions;
void *processConnection(threadOptions *options);
extern "C" {
modInfo *openspy_modInfo();
void *openspy_mod_run(modLoadOptions *options);
bool openspy_mod_query(char *sendmodule, void *data,int len);
}
#endif
