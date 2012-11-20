#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#include <WinSock.h>
typedef int socklen_t;
#define snprintf sprintf_s
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
#include <openspy/structs.h>
#include <common/helpers.h>
#define stricmp strcasecmp
#define sprintf_s snprintf
#define strnicmp strncasecmp
#define KEYPORT 29910
extern "C" {
modInfo *openspy_modInfo();
void *openspy_mod_run(modLoadOptions *options);
bool openspy_mod_query(char *sendmodule, void *data,int len);
}
