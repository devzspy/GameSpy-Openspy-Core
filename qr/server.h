#ifndef _QR_SERVER_INC
#define _QR_SERVER_INC
#include "main.h"
#include "structs.h"
class Client;
void deleteClient(Client *client);
Client *find_user(struct sockaddr_in *peer);
Client *find_user(uint32_t ip, uint16_t port);
countryRegion *findCountryByName(char *name);
#endif
