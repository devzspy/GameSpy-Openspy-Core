
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
typedef int socklen_t;
#define strcasecmp stricmp
#define strncasecmp strnicmp
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
#include <GeoIP.h>
#define stricmp strcasecmp
#define sprintf_s snprintf
#define strnicmp strncasecmp
#endif
#include <fcntl.h>
#include <ctype.h>
#include <list>
#include <deque>
#include <openspy/structs.h>
#include <common/helpers.h>
#include <common/buffreader.h>
#include <common/buffwriter.h>
#include <common/gsmsalg.h>
#include "server.h"
#include "structs.h"
#include <serverbrowsing/structs.h>

#define QRPORT 27900

#define REQUEST_KEY_LEN 4

#define INBUF_LEN 256
#define MAX_DATA_SIZE 1400
#define PUBLIC_ADDR_LEN 12

#define QR_MAGIC_1 0xFE
#define QR_MAGIC_2 0xFD
void checkTimeouts();
extern "C" {
modInfo *openspy_modInfo();
void *openspy_mod_run(modLoadOptions *options);
bool openspy_mod_query(char *sendmodule, void *data,int len);
}