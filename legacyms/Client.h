#ifndef _CLIENT_LEGACYMS_INC
#define _CLIENT_LEGACYMS_INC
#include "main.h"
#define _JUST_STRUCTS
#define _NO_CPP
#include <peerchat/structs.h>
#include <qr/structs.h>
#undef _NO_CPP
#undef _JUST_STRUCTS
class Client {
public:
	Client(int sd, struct sockaddr_in *peer);
	~Client();
	int getSocket();
	struct sockaddr_in *getSockAddr();
	uint32_t getAddress();
	uint16_t getPort();
	time_t getLastPing();
	time_t getConnectTime();
	void processConnection(fd_set *rset);
private:
	void sendCryptHeader();
	void handleData(uint8_t *buff,uint32_t len);
	void handleValidation(uint8_t *buff,uint32_t len);
	void handleList(uint8_t *buff,uint32_t len);
	void sendServers(gameInfo *queryGame, char *filter);
	void sendGroups(gameInfo *queryGame);
	void freeServerRuleList(std::list<customKey *> slist);
	void senddata(char *buff, int len, bool sendFinal = false, bool initPacket = false, bool noEncryption = false, bool actualSend = false);
	int sd;
	struct sockaddr_in sockinfo;
	gameInfo *game;
	uint8_t challenge[7];
	uint8_t enctype;
	time_t connected;
	time_t lastPing;
	bool validated;
	unsigned int cryptkey_enctype2[326];
	unsigned char *keyptr;
//	enctype1_data cryptkey_enctype1;
	char *sendbuff;
	char *sbuffp; //original pointer
	int sbuffalloclen;
	int sendlen;
};
#endif
