#ifndef _QR_STRUCTS_INC
#define _QR_STRUCTS_INC
#define REGIONID_AMERICAS 1
#define REGIONID_NORTH_AMERICA 2
#define REGIONID_CARIBBEAN 4
#define REGIONID_CENTRAL_AMERICA 8
#define REGIONID_SOUTH_AMERICA 16
#define REGIONID_AFRICA 32
#define REGIONID_CENTRAL_AFRICA 64
#define REGIONID_EAST_AFRICA 128
#define REGIONID_NORTH_AFRICA 256
#define REGIONID_SOUTH_AFRICA 512
#define REGIONID_WEST_AFRICA 1024
#define REGIONID_ASIA 2048
#define REGIONID_EAST_ASIA 4096
#define REGIONID_PACIFIC 8192
#define REGIONID_SOUTH_ASIA 16384
#define REGIONID_SOUTH_EAST_ASIA 32768
#define REGIONID_EUROPE 65536
#define REGIONID_BALTIC_STATES 131072
#define REGIONID_CIS 262144
#define REGIONID_EASTERN_EUROPE 524288
#define REGIONID_MIDDLE_EAST 1048576
#define REGIONID_SOUTH_EAST_EUROPE 2097152
#define REGIONID_WESTERN_EUROPE 4194304
class Client;
enum {
	EQRMsgID_GetServer = 32,
	EQRMsgID_GetServerRules,
	EQRMsgID_ClientMessage,
};
typedef struct {
	char *name;
	char *value;
} customKey;
typedef struct {
	customKey key;
	int index; //player 0, team 0, etc
} indexedKey;
typedef struct {
	std::list<Client *> client_list;	
} serverInfo;
typedef struct {
	const char *countrycode;
	const char *countryname;
	uint32_t region;
} countryRegion;
typedef struct {
	std::list<customKey *> serverKeys;
	countryRegion *country;
	uint32_t ipaddr;
	uint16_t port;
} serverList;
typedef struct {
	gameInfo *game;
	uint8_t *filter;
	std::list<serverList> server_list;
	int numServers;
} qrServerList;
typedef struct {
	gameInfo *game;
	uint32_t ipaddr;
	uint16_t port;
	countryRegion *country;
	std::list<customKey *> server_rules;
} qrServerRules;
typedef struct {
	void *data;
	int len;
	uint32_t srcip;
	uint32_t toip;
	uint16_t toport;
} qrClientMsg;
typedef struct {
	uint8_t msgID;
	void *data;
} qrServerMsg;
#endif
