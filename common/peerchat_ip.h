#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

static char peerchat_ip_encdata0[] = "aFl4uOD9sfWq1vGp";
static char peerchat_ip_encdata1[] = "qJ1h4N9cP3lzD0Ka";


unsigned int peerchat_room_decoder(char *data, unsigned int b, unsigned int port);
char *peerchat_room_encoder(unsigned int ip, unsigned int b, unsigned int port);
unsigned int peerchat_ip_decoder(char *data, int encdata_num);
char *peerchat_ip_encoder(unsigned int ip, int encdata_num);
