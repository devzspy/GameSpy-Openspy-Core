#ifndef _SB_SERVER_INC
#define _SB_SERVER_INC
#include "main.h"
class Client;
void deleteClient(Client *client);
Client *find_user(struct sockaddr_in *peer);
Client *find_user(uint32_t ip, uint16_t port);
#endif
