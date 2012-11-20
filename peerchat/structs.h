#ifndef _INC_STRUCT
#define _INC_STRUCT
#ifndef _JUST_STRUCTS
#include "main.h"
#endif
#define MAX_NAME 64
#define MAX_COMMENT 256
#define MAX_CHANS_PER_USER 16
#define WHOWAS_TIMEOUT 3600 //how long until you remove something from the /whowas list
#define QUEUEPORT 33610
#define PINGTIME 80
#define MAX_BUFF 1024
#define MAX_BUFF_DOUBLE MAX_BUFF*2

#ifndef _NO_CPP
class Client;
#else
#endif
enum {
	EQueueID_AddOper = 0x20, //32
	EQueueID_DelOper, //33
	EQueueID_SetUserMode, //34
	EQueueID_DelUserMode, //35
	EQueueID_SetChanProps, //36
	EQueueID_DelChanProps, //37
	EQueueID_AddChanClient, //38
	EQueueID_DelChanClient, //39
	EQueueID_AuthClient, //40
	//Module message IDs
	EMsgID_NumUsersOnChan, //41
};
typedef struct {
	char msgid;
	void *data;
} peerchatMsgData;
typedef struct {
	char channelName[MAX_NAME + 1];
	int numusers; //return value
	#ifdef _NO_CPP
	my_bool
	#else
	bool
	#endif
	showInvisible;
} msgNumUsersOnChan;
typedef struct {
	int profileid;
	uint32_t rightsmask;
} msgAddOper;
typedef struct {
	int profileid;
} msgDelOper;
enum EModeFlags {
	EModeFlags_None = 0,
	EModeFlags_Voice = 1 << 0,
	EModeFlags_HalfOp = 1 << 1,
	EModeFlags_Op = 1 << 2,
	EModeFlags_Owner = 1 << 3,
	EModeFlags_Gag = 1 << 4,
	EModeFlags_Ban = 1 << 5,
	EModeFlags_BanExcempt = 1 << 6,
	EModeFlags_Invited = 1 << 7,
};
typedef struct {
	char chanmask[MAX_NAME + 1];
	char comment[MAX_COMMENT + 1];
	time_t expires;
	char hostmask[MAX_NAME + 1];
	char machineid[MAX_NAME + 1];
	int modeflags;
	int profileid;
	char setbyhost[MAX_NAME + 1];
	char setbynick[MAX_NAME + 1];
	int setbypid;
	int usermodeid;
	char isGlobal; //is it temp or stored in db
	char hideNick; //only used while setting it(remove when we use the db msgs to do this)
	time_t setondate;
} userMode;
typedef struct {
	char chankey[MAX_NAME + 1];
	char chanmask[MAX_NAME + 1];
	char comment[MAX_COMMENT + 1];
	char entrymsg[MAX_COMMENT + 1];
	time_t expires;
	char groupname[MAX_NAME + 1];
	int limit;
	char modes[MAX_NAME + 1];
	char onlyowner;
	char setbynick[MAX_NAME + 1];
	char setbyhost[MAX_NAME + 1];
	int setbypid;
	char topic[MAX_COMMENT + 1];
	time_t setondate;
} chanProps;
typedef struct {
	char chanmask[MAX_NAME + 1];
	int gameid;//-1 = all encrypted, 0 = unencrypted or anything, otherwise specific gameid
} chanGameClient;
typedef struct {
	chanProps props;
	char kickexisting;
} msgSetChanProps;
typedef struct {
	char chanmask[MAX_NAME + 1];
	char kickexisting;
} msgDelChanProps;
typedef struct {
	int userid;
	int profileid;
	int sd;
	char uniquenick[MAX_NAME + 1];
	char sendnotice;
} msgAuthClient;
#ifndef _NO_CPP
typedef struct {
	char name[MAX_COMMENT + 1];
	char value[MAX_COMMENT + 1];
} customKey;
typedef struct {
	char name[MAX_NAME + 1];
	char username[MAX_NAME + 1];
	char host[MAX_NAME + 1];
	char realname[MAX_NAME + 1];
	time_t quittime;
} whowasInfo;
typedef struct {
	int profileid;
	uint32_t rightsmask;
} operInfo;
typedef struct {
	Client *invited;
	Channel *chan;
} inviteInfo;
typedef struct {
	std::list<Client *> client_list;
	std::list<Channel *> chan_list;
	std::list<userMode *> usermodes_list;
	std::list <chanProps *> chanprops_list;
	std::list <whowasInfo *> whowas_list;
	gameInfo *games;
	int num_games;
	std::list<operInfo *> oper_list;
	std::list<inviteInfo *> invite_list;
	std::list<chanGameClient *> chanclient_list;//allowed games into a given channel
	MYSQL *conn;
}peerchatServer;
typedef struct {
	int sd;
	struct  sockaddr_in peer;
	Client *client;
	peerchatServer *server;
} clientParams;
typedef struct {
	Client *client;
	bool gag;
	bool voice;
	bool halfop;
	bool op;
	bool owner;
	bool invisible;
	bool quiet;
	bool invited;
	std::list <customKey *> *userKeys;
} chanClient;
#endif
#endif
