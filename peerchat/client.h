#ifndef _CLIENT_INC
#define _CLIENT_INC
#include "main.h"
#include "server.h"
#include <common/gs_peerchat.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#include "structs.h"
class Client;
enum ECmdRegister {
	CMDREGISTER_EITHER = 0,
	CMDREGISTER_NO,
	CMDREGISTER_YES,
};
enum EOperPrivileges {
	OPERPRIVS_NONE = 0,
	OPERPRIVS_INVISIBLE = 1<<0,
	OPERPRIVS_BANEXCEMPT = 1<<1,
	OPERPRIVS_GETOPS = 1<<2,
	OPERPRIVS_GLOBALOWNER = 1<<3,
	OPERPRIVS_GETVOICE = 1<<4,
	OPERPRIVS_OPEROVERRIDE = 1<<5,
	OPERPRIVS_WALLOPS = 1<<6,
	OPERPRIVS_KILL = 1<<7,
	OPERPRIVS_FLOODEXCEMPT = 1<<8,
	OPERPRIVS_LISTOPERS = 1<<9,
	OPERPRIVS_CTCP = 1<<10, //and ATM, etc
	OPERPRIVS_HIDDEN = 1 << 11,
	OPERPRIVS_SEEHIDDEN = 1 << 12,
	OPERPRIVS_MANIPULATE = 1 << 13, //can manipulate other peoples keys, etc
	OPERPRIVS_SERVMANAGE = 1 << 14,
	OPERPRIVS_WEBPANEL = 1 << 15, //permitted to log in to the web panel(doesn't serve any use on the actual server)
};
enum EUserMode {
	EUserMode_None = 0,
	EUserMode_Quiet = 1 << 0,
	EUserMode_ShowConns = 1 << 1, //oper mode(OPERPRIVS_LISTOPERS)
	EUserMode_ShowJoins = 1 << 2, //oper mode(OPERPRIVS_LISTOPERS)
	EUserMode_SpyMessages = 1 << 3, //oper mode(OPERPRIVS_LISTOPERS)
	EUserMode_HideSpyMessages = 1 << 4, //oper mode(OPERPRIVS_MANIPULATE)
	EUserMode_AllowInvisiblePrivmsg = 1 << 5, //oper mode, allows invisible users to send privmsgs(OPERPRIVS_INVISIBLE)
};
typedef struct {
	char *name;
	EOperPrivileges rightsmask;
	ECmdRegister registered;
	int flood;//how much it increases your flood weight for each time you do this command
	bool (Client::*mpFunc)(char *);

}commandInfo;
class Client {
public:
	Client(clientParams* params);
	~Client();
	int sendToClient(char *fmt, ...);
	void getUserInfo(char **nick, char **user, char **host, char **realname);
	void send_numeric(short num, char *fmt, ...);
	void messageSend(Client *from, char *fmt, ...);
	int sendToClientQuiet(char *fmt, ...);
	uint32_t getRights();
	void quitUser(char *reason);
	time_t getLastreply();
	time_t getConnectedTime();
	void mainLoop(fd_set *rset);
	int getProfileID();
	bool isGagged();
	void togGag();
	void setGagged(bool value);
	bool checkQuit();
	bool getUserKey(char *name, char *dst,int len);
	int getSocket();
	int getModeFlags();
	int getGameID();
	uint32_t getIP();
	void logUserIn(int userid, int profileid, bool sendnotice = true);
	bool waiting_ping;
	time_t last_ping;
	char uniquenick[MAX_NAME];//unique name for /nick *(killing duplicate names)
private:
	void parseIncoming();
	void parseIncoming(char *buff, int len);
	bool cmd_user(char *params);
	bool cmd_nick(char *params);
	bool cmd_wallops(char *params);
	bool cmd_setrights(char *params);
	bool cmd_opermsg(char *params);
	bool cmd_listopers(char *params);
	bool cmd_listusers(char *params);
	bool cmd_quit(char *params);
	bool cmd_pingme(char *params);
	bool cmd_pong(char *params);
	bool cmd_join(char *params);
	bool cmd_part(char *params);
	bool cmd_names(char *params);
	bool cmd_mode(char *params);
	bool cmd_privmsg(char *params);
	bool cmd_topic(char *params);
	bool cmd_whois(char *params);
	bool cmd_setusermode(char *params);
	bool cmd_listusermodes(char *params);
	bool cmd_delusermode(char *params);
	bool cmd_kill(char *params);
	bool cmd_kline(char *params);
	bool cmd_userhost(char *params);
	bool cmd_setchanprops(char *params);
	bool cmd_listchanprops(char *params);
	bool cmd_delchanprops(char *params);
	bool cmd_setgroup(char *params);
	bool cmd_listchans(char *params);
	bool cmd_whowas(char *params);
	bool cmd_setkey(char *params);
	bool cmd_getkey(char *params);
	bool cmd_getchankey(char *params);
	bool cmd_setchankey(char *params);
	bool cmd_getckey(char *params);
	bool cmd_setckey(char *params);
	bool cmd_kick(char *params);
	bool cmd_who(char *params);
	bool cmd_match(char *params);
	bool cmd_cdkey(char *params);
	bool cmd_crypt(char *params);
	bool cmd_oper(char *params);
	bool cmd_invite(char *params);
	bool cmd_ison(char *params);
	bool cmd_away(char *params);
	bool cmd_notice(char *params);
	bool cmd_atm(char *params);
	bool cmd_utm(char *params);
	bool cmd_login(char *params);
	bool cmd_fjoin(char *params);
	bool cmd_lusers(char *params);
	void reduceWeight();
	void sendWhois(Client *target);
	void sendNames(char *chan);
	void sendModes(Client *who);
	bool joinChan(char *name, chanClient *userInfo, char *key);
	bool partChan(char *name, char *reason);
	void msgSend(char *name,bool no_ctcp, char *params);
	void sendUserInfo(Client *target, char *cmd);
	void sendWelcome();
	void setModes(char *modestr);
	bool addUserParam(customKey *key);
	customKey *getParamValue(char *name, char *dst, int dstlen);
	int sd;
	struct  sockaddr_in peer;
	gs_peerchat_ctx eClient,eServer;
	bool encryted; //used in favour of checking if gameid is zero to support gmtest
	int gameid;
	char nick[MAX_NAME];
	char user[MAX_NAME];
	char realname[MAX_NAME];
	char host[MAX_NAME];
	char buff[MAX_BUFF_DOUBLE];//buff for everything(in/out)
	int len;
	bool gagged;
	bool registered;
	bool toquit;
	int weight;
	time_t lastWeightReduction;
	uint32_t modeflags;
	char awaytext[MAX_COMMENT];
	char quitreason[MAX_COMMENT];
	uint32_t rightsmask;
	time_t connected;
	time_t last_reply;
	bool welcomed;
	std::list<customKey *> userKeys;//setkey/getkey
	int profileid;
};
#endif
