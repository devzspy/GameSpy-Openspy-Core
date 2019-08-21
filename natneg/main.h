#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <time.h>
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
#include <list>
#include <openspy/structs.h>
#include <common/helpers.h>
#define NN_PING_TIME 120
#define NN_DEADBEAT_TIME 30
int getIP(int instance);
int getProbeIP();
extern "C" {
modInfo *openspy_modInfo();
void *openspy_mod_run(modLoadOptions *options);
bool openspy_mod_query(char *sendmodule, void *data,int len);
}
