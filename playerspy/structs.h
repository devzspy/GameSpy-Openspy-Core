#ifndef _INC_GP_STRUCTS
#define _INC_GP_STRUCTS
#define MAX_BUFF 1024
#include "main.h"
class Client;
typedef struct {
	std::list<Client *> client_list;
	gameInfo *games;
	int num_games;
	MYSQL *conn;
	modLoadOptions *options;
}playerspyServer;
typedef struct {
	int sd;
	struct  sockaddr_in peer;
} clientParams;
#endif
