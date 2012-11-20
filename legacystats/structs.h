#ifndef _INC_LS_STRUCTS
#define _INC_LS_STRUCTS
#define MAX_BUFF 4096
#include "main.h"
class Client;
typedef struct {
	std::list<Client *> client_list;
	modLoadOptions *options;
	MYSQL *conn;
}legacyStatsServer;
typedef struct {
	int sd;
	struct  sockaddr_in peer;
} clientParams;
typedef struct {
	char *name;
	char *value;
} gameKey;
/********
persisttype_t
There are 4 types of persistent data stored for each player:
pd_private_ro: Readable only by the authenticated client it belongs to, can only by set on the server
pd_private_rw: Readable only by the authenticated client it belongs to, set by the authenticated client it belongs to
pd_public_ro: Readable by any client, can only be set on the server
pd_public_rw: Readable by any client, set by the authenicated client is belongs to
*********/
typedef enum {pd_private_ro, pd_private_rw, pd_public_ro, pd_public_rw} persisttype_t;
#endif
