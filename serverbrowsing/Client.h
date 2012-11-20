#ifndef _HANDLER_SERVERBROWSING_INC
#define _HANDLER_SERVERBROWSING_INC
#include "main.h"
#define _JUST_STRUCTS
#define _NO_CPP
#include <peerchat/structs.h>
#include <qr/structs.h>
#undef _NO_CPP
#undef _JUST_STRUCTS
#define CRYPTCHAL_LEN 10
#define SERVCHAL_LEN 25
class Client {
public:
	Client(int sd, struct sockaddr_in peer);
	~Client();
	int getSocket();
	struct sockaddr_in *getSockAddr();
	uint32_t getAddress();
	time_t getLastPing();
	void processConnection(fd_set *rset);
	bool wantsUpdates();
	gameInfo *getQueryGame();
	uint8_t *getFilter();
	void pushServer(serverList slist);
	void delServer(serverList slist);
private:
	int handleListRequest(uint8_t *buff, uint32_t len);
	int handleInfoRequest(uint8_t *buff, uint32_t len);
	int handleMessageRequest(uint8_t *buff, uint32_t len);
	void sendGroups();
	void sendIP();
	void handleData(uint8_t *buff, uint32_t len);
	void addGroupBuff(char **buff,int *len, char *fieldList, MYSQL_ROW row);
	void sendServers();
	char *findServerValue(char *name,serverList list);
	void addServerBuff(char **buff,int *len, serverList slist);
	void setupCryptHeader( uint8_t **dst, uint32_t *len);
	void InitCryptKey(char *challenge, char *queryfromkey,char *key, int keylen);
	void sendKeyList();
	void sendServerRules(std::list<customKey *> server_rules,uint32_t ip, uint16_t port);
	int getNumberOfServers(uint16_t groupid);
	void freeServerRuleList(std::list<customKey *> slist);
	uint8_t *querygame;
	uint8_t *gamename;
	uint8_t *filter;
	uint8_t *fieldlist;
	uint8_t challenge[9];
	uint32_t srcip;
	uint32_t maxservers;
	uint32_t fromgamever;
	uint8_t listversion;
	uint8_t encodingversion;
	uint32_t options;
	uint32_t fromip;
	int sd;
	struct sockaddr_in peer;

	uint32_t msgIP;
	uint16_t msgPort;
	bool nextMsgMsgReq;
	unsigned char   encxkeyb[261];
	time_t lastKeepAlive;
	gameInfo *game;
	gameInfo *queryGame;
	bool	cryptHeaderSent;
	uint32_t headerLen;
};
#endif
