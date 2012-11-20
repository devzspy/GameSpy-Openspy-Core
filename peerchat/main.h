#ifndef _MAIN_INC
#define _MAIN_INC
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
	#include <Windows.h>
	#include <WinSock.h>
	typedef int ssize_t;
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
    #include <fcntl.h>
    #include <ctype.h>
    #include <stropts.h>
#endif
#include <openspy/structs.h>
#include <common/helpers.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <common/mysql_helpers.h>
#include <stdarg.h>
#include <time.h>
#include <deque>
#include <queue>
#include <vector>
#include <list>
#include <assert.h>
#include "client.h"
#ifdef WIN32
#define close closesocket
#define sleep Sleep
#define snprintf sprintf_s
#endif
#endif
class Client;
class Channel;
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#pragma warning(disable:4018)
bool do_db_check();
extern "C" { 
void *openspy_mod_run(modLoadOptions *options);
modInfo *openspy_modInfo();
bool openspy_mod_query(char *sendmodule, void *data,int len);
}
