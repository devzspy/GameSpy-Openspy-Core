#ifndef _CLIENT_GP_INC
#define _CLIENT_GP_INC
#include "main.h"
#include "structs.h"
#include "server.h"
#include <common/gs_login_proof.h>
extern playerspyServer server;
class Client {
public:
	Client(clientParams* params);
	~Client();
	void mainLoop(fd_set *rset);
	int getSocket();
	int getProfileID();
	int getUserID();
	bool hasBuddy(Client *c);
	bool hasBuddy(int upid);
	bool hasBlocked(Client *c);
	bool hasBlocked(int upid);
	void sendBuddyStatus(Client *c);
	bool productInviteable(int productid);
	time_t getLastPacket();
private:
	void parseIncoming();
	void parseIncoming(char *buff, int len);
	void handleLogin(char *buff, int len);
	void handleNewUser(char *buff, int len);
	void handleStatus(char *buff, int len);
	void handleGetProfile(char *buff, int len);
	void handleAddBuddy(char *buff, int len);
	void handleAuthAdd(char *buff, int len);
	void handleDelBuddy(char *buff, int len);
	void handleBM(char *buff, int len);
	void handleUpdateProfile(char *buff, int len);
	void handleRevoke(char *buff, int len);
	void handleNewProfile(char *buff, int len);
	void handleDelProfile(char *buff, int len);
	void handleAddBlock(char *buff, int len);
	void handleRemoveBlock(char *buff, int len);
	void handleInviteTo(char *buff, int len);
	void loadBuddies();
	void loadBlockedList();
	void handlePlayerInvite(char *buff, int len);
	void handleRegisterNick(char *buff, int len);
	void handleUpdateUI(char *buff, int len);
	void sendAddRequests();
	void sendBuddies();
	void deleteBuddy(int profileid);
	void deleteBlock(int profileid);
	void sendBuddyInfo(int profileid);
	void saveMessage(int profileid,char *msg);
	void sendMessages();
	void sendError(int sd, bool fatal, char *msg, GPErrorCode errid, int id);
	int sd;
	int profileid;
	int userid;
	char challenge[11];
	char buff[MAX_BUFF];
	char email[GP_EMAIL_LEN+1];
	int len;
	int sesskey;
	char statusstr[GP_STATUS_STRING_LEN];
	char locstr[GP_STATUS_STRING_LEN];
	std::list<int> buddies;
	std::list<int> blocklist;
	std::list<int> inviteableProducts;
	GPEnum status;
	int sdkrevision;
	bool sentBuddies;
	bool sentAddRequests;
	int port; // \port\ command from \login
	int ip;
	GPEnum quietflags;
	time_t lastPacket;
	gameInfo *game;
	
	
};
#endif
