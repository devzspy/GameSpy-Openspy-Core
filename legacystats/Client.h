#ifndef _CLIENT_GP_INC
#define _CLIENT_GP_INC
#include "main.h"
#include "structs.h"
#include "server.h"
#include <common/gs_login_proof.h>
#include <common/gs_chrep_num.h>
#include <common/gs_sesskey.h>
extern legacyStatsServer server;
class Client {
public:
	Client(clientParams* params);
	~Client();
	void mainLoop(fd_set *rset);
	int getSocket();
	int getProfileID();
	int getUserID();
	time_t getLastPacket();
private:
	void parseIncoming();
	void parseIncoming(char *buff, int len);
	void handleAuth(char *buff,int len);
	void handleAuthP(char *buff,int len);
	void handleGetPD(char *buff,int len);
	void handleSetPD(char *buff,int len);
	void handleUpdateGame(char *buff,int len);
	void getResponse(int chrespnum, char *secretkey, char *out, int dstlen);
	void getLoginResponse(char *sesskey, char *pass, char *out, int dstlen);
	void sendError(int sd, bool fatal, char *msg, GPErrorCode errid, int id);
	char *mergeKeys(char *keys,int localid,persisttype_t type,int index,int pid);
	bool shouldUpdateData(int dindex, persisttype_t type, int pid);
	int tryCdKeyLogin(char *keyhash);
	void saveGame(bool done = true);
	void clearGameKeys();
	int getKeyStrSize();
	int sd;
	int profileid;
	int userid;
	bool gameInProgress;
	char challenge[11];
	char buff[MAX_BUFF];
	char email[GP_EMAIL_LEN+1];
	std::list<gameKey *> gameKeys;
	int len;
	int sesskey;
	int port; // \port\ command from \login
	int ip;
	gameInfo *game;
	bool authenticated; //passed first challenge
	time_t lastPacket;
	
	
};
#endif
