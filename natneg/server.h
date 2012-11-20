#ifndef _NN_SERVER_INC
#define _NN_SERVER_INC
#include "main.h"
#include "structs.h"
class Client;
typedef struct {
	modLoadOptions *loadOptions;
	std::list<Client *> client_list;	
} serverInfo;
void deleteClient(Client *client);
Client *find_user(struct sockaddr_in *peer, int instance);
Client *find_user(uint32_t ip, uint16_t port, int instance);
Client *find_user_by_cookie(int cookie, int instance);
Client *find_user_by_cookie_index(int cookie, int instance, int index);
#endif
