#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#include <WinSock.h>
#define snprintf sprintf_s
#define close closesocket
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
#include <stdarg.h>
#include <time.h>
#include <deque>
#include <queue>
#include <vector>
#include <list>
#include <assert.h>
#define stricmp strcasecmp
#define sprintf_s snprintf
#define strnicmp strncasecmp
#define GPCM_PORT 29900
#include <openspy/structs.h>
#include <common/helpers.h>
#include <common/mysql_helpers.h>
#include "gp.h"
#include "structs.h"
extern MYSQL *conn;
extern MYSQL_RES *res;
extern MYSQL_ROW row;
void *processConnection(void *param);
bool do_db_check();
extern "C" {
modInfo *openspy_modInfo();
void *openspy_mod_run(modLoadOptions *options);
bool openspy_mod_query(char *sendmodule, void *data,int len);
}
