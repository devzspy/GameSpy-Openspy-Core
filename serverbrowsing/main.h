#ifndef _MAIN_SERVERBROWSING_INC
#define _MAIN_SERVERBROWSING_INC
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#include <WinSock.h>
typedef int socklen_t;
#define close closesocket
#define MSG_DONTWAIT 0
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/times.h>
#include <stropts.h>
#define stricmp strcasecmp
#define sprintf_s snprintf
#define strnicmp strncasecmp
#endif
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <list>
#include <deque>
#define SBPORT 28910
#define SB_TIMEOUT_TIME 120
#include <openspy/structs.h>
#include <common/helpers.h>
#include <common/buffreader.h>
#include <common/buffwriter.h>
#include <common/enctypex_decoder.h>
#include "structs.h"
extern MYSQL *conn;
extern modLoadOptions servoptions;
extern modInfo moduleInfo;
typedef struct {
	int sd;
	struct  sockaddr_in peer;
}threadOptions;
void *processConnection(threadOptions *options);
bool do_db_check();
extern "C" {
modInfo *openspy_modInfo();
void *openspy_mod_run(modLoadOptions *options);
bool openspy_mod_query(char *sendmodule, void *data,int len);
}
#endif
