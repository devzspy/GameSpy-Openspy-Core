#ifndef _CHAN_INC
#define _CHAN_INC
#include "server.h"
#ifndef PINGTIME
#define PINGTIME 120
#endif
enum ENotEnoughParams {
	ENotEnough_HALFOP,
	ENotEnough_OP,
	ENotEnough_OWNER,
};
class Channel {
public:
	Channel(char *name);
	~Channel();
	char *getName();
	void addUser(chanClient *user);
	void removeUser(Client *user, bool sendpart);
	bool userOn(Client *user);
	chanClient *getUserInfo(Client *user);
	void sendToChanExcept(Client *exception, char *fmt, ...);
	void invisibleSend(char *fmt, ...);//send to those who can see invisible users;
	void noninvisibleSend(char *fmt,...);
	void sendToChan(char *fmt,...);
	void sendMessage(bool invisible,bool skip_sender, Client *sender, char *fmt, ...);
	int getNumUsers(bool showinvisible = false);
	void sendNames(Client *who);
	void sendModes(Client *who);
	void setModes(Client *setter, char *modestr);
	void setTopic(Client *setter, char *str);
	void sendTopic(Client *who);
	void senduserModes(chanClient *who);
	void sendNotEnoughPrivs(ENotEnoughParams type, Client *who, const char *comment);
	void applyChanProps(chanProps *props,bool kickexisting);
	void removeUser(Client *user, bool sendpart, char *reason, bool closeChan = true);
	void resetChannel(bool deleteatend = false, bool removed = false);
	void clearProps();
	bool getChanKey(char *name, char *dst,int len, bool skip_userset = false);
	char *getKeyBuff(); //allocates memory for sending keybuff, free after..
	customKey *getParamValue(char *name, char *dst, int dstlen);
	bool addParam(customKey* key);
	bool addUserParam(customKey* key, chanClient *user);
	void clearnonGlobalModes();
	std::list<chanClient *> getList();
	bool moderated;
	bool inviteonly;
	int limit;
	bool stayopen;
	bool nooutsidemsgs;
	bool topic_protect;
	bool priv;
	bool secret;
	bool onlyowner;
	bool registered;//+r
	bool allops;//+a
	bool mode_ops_obey_channel_limit;
	char auditorium;//0 = off, 1 = see ops/voice(q), 2 = see no one(u)
	char key[MAX_NAME];
	char groupname[MAX_NAME];
	char topic[MAX_COMMENT];
	char topicsetter[MAX_NAME];
	char entrymsg[MAX_COMMENT];
	time_t topictime;
	time_t createtime;
	int setup;//will be 0xABCDEF02 if channel set up
	std::list<customKey *> chanKeys;//setkey/getkey
private:
	void auditoriumUpdate(char oldMode);
	char name[MAX_NAME];
	std::list<chanClient *> user_list;
};
#endif
