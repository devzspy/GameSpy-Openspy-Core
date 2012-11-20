#include "main.h"
#include "server.h"
#include "channel.h"
extern peerchatServer server;
void deleteClient(Client *client) {
	std::list<Client *>::iterator iterator;
	iterator=server.client_list.begin();
	while(iterator != server.client_list.end()) {
		if(*iterator==client) {
			iterator = server.client_list.erase(iterator);
			delete client;
		} else
		iterator++;

	}
}
void deleteChannel(Channel *client) {
	std::list<Channel *>::iterator iterator;
	iterator=server.chan_list.begin();
	while(iterator != server.chan_list.end()) {
		if(*iterator==client) {
			iterator = server.chan_list.erase(iterator);
			delete client;
		} else
		iterator++;

	}
}
void sendToAll(char *str, ...) {
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, str );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),str,args);
	va_end(args);
	std::list<Client *> list = server.client_list;
	std::list<Client *>::iterator iterator;
	iterator=list.begin();
	Client *c;
	while(iterator != list.end()) {
		c=*iterator;
		c->sendToClient(str);
		iterator++;
	}
}
void sendWallops(Client *sender,char *msg) {
	char sbuff[MAX_BUFF];
	char *nick,*user,*cnick;
	memset(&sbuff,0,sizeof(sbuff));
	std::list<Client *> list = server.client_list;
	std::list<Client *>::iterator iterator;
	iterator=list.begin();
	Client *c;
	while(iterator != list.end()) {
		c=*iterator;
		sender->getUserInfo(&nick,&user,NULL,NULL);
		c->getUserInfo(&cnick,NULL,NULL,NULL);
		sprintf_s(sbuff,sizeof(sbuff),":%s!%s@* PRIVMSG %s :GLOBAL MESSAGE: %s",nick,user,cnick,msg);
		c->sendToClient(sbuff);
		iterator++;
	}
}
void sendToAllOpers(uint32_t rights,char *str,...) {
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, str );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),str,args);
	va_end(args);
	std::list<Client *> list = server.client_list;
	std::list<Client *>::iterator iterator;
	iterator=list.begin();
	Client *c;
	while(iterator != list.end()) {
		c=*iterator;
		if(!(c->getRights() & rights)) {
			c->sendToClient(sbuff);
		}
		iterator++;
	}
}
void sendToAllOpers(char *str,...) {
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, str );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),str,args);
	va_end(args);
	std::list<Client *> list = server.client_list;
	std::list<Client *>::iterator iterator;
	iterator=list.begin();
	Client *c;
	while(iterator != list.end()) {
		c=*iterator;
		if((c->getRights()  != 0)) {
		c->sendToClient(sbuff);
		}
		iterator++;
	}
}
/* bool Client::checkPing() {
	 int timeaa=(int)time(NULL);
	 //timeaa+=PINGTIME;

	 if(last_replay<timeaa) {
		 if(!waiting_ping) {
			sendToClient("PING: s");
			waiting_ping = true;
		 }
	 }
	 printf("%i %i\n",last_replay,timeaa);
	 //if(last_replay<time(NULL)+500) {
		// quitUser("Ping Timeout");
	 //}
	 return true;
 }*/
bool nameInUse(char *name) {
	Client *c;
	char *cnick;
	std::list<Client *>::iterator iterator=server.client_list.begin();
		 while(iterator != server.client_list.end()) {
			c=*iterator;
			c->getUserInfo(&cnick,NULL,NULL,NULL);
			if(!stricmp(cnick,name)) {
				return true;
			}
			iterator++;
		 }
	return false;
}
bool chanCharValid(char ch) {
char allowed[]=
		"1234567890"
		"_-`()$-=;/"
		"@+&%!#"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for(int i=0;i<sizeof(allowed);i++) {
		if(allowed[i]==ch) return true;
	}
	return false;
}
bool validChan(char *name) {
	if(name[0] != '#') return false;
	for(int i=0;i<strlen(name);i++) {
		if(!chanCharValid(name[i])) { //maybe check into it?
			return false;
		}
	}
	return true;
}
Channel *find_chan(char *name) {
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	Channel *chan;
	char *cname;
	while(iterator != server.chan_list.end()) {
		chan=*iterator;
		cname=chan->getName();
		if(stricmp(name,cname)==0) {
			return chan;
		}
		iterator++;
	}
	return NULL;
}
Client *find_user(char *name) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *chan;
	char *cname;
	while(iterator != server.client_list.end()) {
		chan=*iterator;
		chan->getUserInfo(&cname,NULL,NULL,NULL);
		if(stricmp(name,cname)==0) {
			return chan;
		}
		iterator++;
	}
	return NULL;
}
bool canSeeIp(Client *s,Client *target) {
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	Channel *chan;
	chanClient *c_client;
	if(s->getRights()&OPERPRIVS_GLOBALOWNER) return true;
	while(iterator != server.chan_list.end()) {
		chan=*iterator;
		if(chan->userOn(s)&&chan->userOn(target)) {
			c_client=chan->getUserInfo(s);
			if(c_client->halfop||c_client->op||c_client->owner) {
				if(~target->getRights()&OPERPRIVS_GLOBALOWNER) {
					return true;
				}
			}
		}
		iterator++;
	}
	return false;
}
bool onChanTogether(Client *s, Client *target) {
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	Channel *chan;
	while(iterator != server.chan_list.end()) {
		chan=*iterator;
		if(chan->userOn(s)&&chan->userOn(target)) {
				return true;
		}
		iterator++;
	}
	return false;
}
bool onQueue(Client *who, std::deque<Client *> queue) {
	std::deque<Client *>::iterator it=queue.begin();
	while(it != queue.end()) {
		if(((Client *)(*it))==who) return true;
		it++;
	}
	return false;
}
void sendMessageQueue(Client *source,std::deque<Client *> queue, char *buff) {
	std::deque<Client *>::iterator it=queue.begin();
	while(it != queue.end()) {
		(*it)->messageSend(source,buff);
		it++;
	}
}
void sendOnceAllChan(Client *source, bool sendtosource,char *str,...) {
	std::deque<Client *> sendqueue;
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	std::list<chanClient *>::iterator it;
	std::list<chanClient *> chanClient_list;
	chanClient *currentUser,*c_Client;
	Channel *chan;
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, str );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),str,args);
	va_end(args);
	while(iterator != server.chan_list.end()) {
		chan=*iterator;
		if(chan->userOn(source)) {
			chanClient_list = chan->getList();
			currentUser = chan->getUserInfo(source);
			it = chanClient_list.begin();
			while(it != chanClient_list.end()) {
				c_Client = *it;
				if(!sendtosource && c_Client->client == source) { it++; continue; }
				if(!currentUser->invisible || c_Client->client->getRights()&OPERPRIVS_INVISIBLE) {
					if(!onQueue(c_Client->client,sendqueue)) {
						sendqueue.push_back(c_Client->client);
					}
				}
				it++;
			}
		}
		iterator++;
	}
	sendMessageQueue(source,sendqueue,sbuff);
}
int numChans(Client *user) {
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	Channel *chan;
	int i=0;
	while(iterator != server.chan_list.end()) {
		chan=*iterator;
		if(chan->userOn(user)) {
				i++;
		}
		iterator++;
	}
	return i;
}
void removeFromAllChans(Client *user) {
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	Channel *chan;
	int i=0;
	while(iterator != server.chan_list.end()) {
		chan=*iterator;
		if(chan->userOn(user)) {
			chan->removeUser(user,false);
			iterator=server.chan_list.begin(); //because the channel might have been deleted
			continue;
			/*
			if(chan->getNumUsers() == 0) {
				if(!chan->stayopen) {
					delete chan;
					//iterator=server.chan_list.begin();
					continue;
				}
			}
			*/
		}
		iterator++;
	}
}
EModeFlags getModeStr(char *str) {
	int modeflags = 0;
	int len = strlen(str);
	for(int i=0;i<len; i++) {
		switch(str[i]) {
		case 'v':
			modeflags |= EModeFlags_Voice;
			break;
		case 'h':
			modeflags |= EModeFlags_HalfOp;
			break;
		case 'o':
			modeflags |= EModeFlags_Op;
			break;
		case 'O':
			modeflags |= EModeFlags_Owner;
			break;
		case 'b':
			modeflags |= EModeFlags_Ban;
			break;
		case 'g':
			modeflags |= EModeFlags_Gag;
			break;
		case 'I':
			modeflags |= EModeFlags_Invited;
			break;
		case 'E': 
			modeflags |= EModeFlags_BanExcempt;
			break;
		}
	}
	return (EModeFlags)modeflags;
}
void addUserMode(Client *setter, char *target, char *modestr, bool addSQL) {
	char data[128];
	char *nick, *host;
	int pid = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;
	userMode *usermode = (userMode *)malloc(sizeof(userMode));
	if(usermode == NULL) return;
	memset(usermode,0,sizeof(userMode));
	usermode->isGlobal = true;
	if(find_param("hostmask", modestr, data, MAX_NAME)) {
		strcpy(usermode->hostmask,data);
	}
	if(find_param("comment", modestr, data, sizeof(data))) {
		strcpy(usermode->comment,data);
	}
	if(find_param("machineid", modestr, data, MAX_NAME)) {
		strcpy(usermode->machineid, data);
	}
	if(find_param("profileid", modestr, data, MAX_NAME)) {
		usermode->profileid = atoi(data);
	}
	if(find_param("modeflags", modestr, data, sizeof(data))) {
		usermode->modeflags = getModeStr(data);
	} else {
		free((void *)usermode);
		return;
	}
	if(find_param("hidenick", modestr, data, sizeof(data))) {
		usermode->hideNick = atoi(data)==1?true:false;
	} else {
		usermode->hideNick = false;
	}
	if(find_param("expiressec", modestr, data, MAX_NAME)) {
		usermode->expires = atoi(data) + time(NULL);
	}
	strcpy(usermode->chanmask,target);
	usermode->setondate = time(NULL);
	if(setter != NULL) {
		pid = setter->getProfileID();
		setter->getUserInfo(&nick,NULL,&host,NULL);
		strcpy(usermode->setbynick, nick);
		usermode->setbypid = pid;
		strcpy(usermode->setbyhost,host);
	} else {
		usermode->setbypid = 0;
		strcpy(usermode->setbynick,"SERVER");
	}
	if(setter != NULL && ~setter->getRights() & OPERPRIVS_GLOBALOWNER) {
		Channel *chan = find_chan(target);
		if(chan == NULL) {
			setter->send_numeric(403, "%s %s :No such channel",nick,target);
			return;
		}
		chanClient *cClient = chan->getUserInfo(setter);
		if(cClient == NULL) {
			//:s 442 CHC #gsp!subhome :You're not on that channel
			setter->send_numeric(442, "%s %s :You're not on that channel",nick,target);
			return;
		}
		if(chan->onlyowner) {
			if(!cClient->owner) {
				chan->sendNotEnoughPrivs(ENotEnough_OWNER,setter,"Setting ops");
				return;
			}
		}
		if(usermode->modeflags & EModeFlags_Op && !cClient->owner) {
			chan->sendNotEnoughPrivs(ENotEnough_OWNER,setter,"Setting ops");
			return;
		}
		if(usermode->modeflags & EModeFlags_HalfOp && !cClient->op && !cClient->owner) {
			chan->sendNotEnoughPrivs(ENotEnough_OP,setter,"Setting Halfops");
			return;
		}
		if(usermode->modeflags & EModeFlags_Op && !cClient->op && !cClient->owner) {
			chan->sendNotEnoughPrivs(ENotEnough_OP,setter,"Setting ops");
			return;
		}
		if(usermode->modeflags & EModeFlags_Voice && !cClient->op && !cClient->owner && !cClient->halfop) {
			chan->sendNotEnoughPrivs(ENotEnough_HALFOP,setter,"Setting voice");
			return;
		}
		if(cClient->halfop == false && cClient->op == false && cClient->owner == false) {
			chan->sendNotEnoughPrivs(ENotEnough_HALFOP,setter,"Setting modes");
			return;
		}
		if(chan->registered) {
			usermode->isGlobal = 0;
		}
	}
	if(setter == NULL || setter->getRights() & OPERPRIVS_GLOBALOWNER) {
		if(find_param("isglobal", modestr, data, MAX_NAME)) {
			usermode->isGlobal = atoi(data)==1?1:0;
		}
	}
	//if(usermode->isGlobal == 0) {
		//usermode->usermodeid = server.usermodes_list.size();
	//	usermode->usermodeid -= (usermode->usermodeid * 2);
//	} else {
	usermode->usermodeid = server.usermodes_list.size()+1;
	//}
	if(usermode->isGlobal == 1) {
		int len = sizeof(userMode) * 2;
		char *query = (char *)malloc(len);
	#define sqlEscapeProp(name)  char name[sizeof(usermode->name)];\
 mysql_real_escape_string(server.conn,name,usermode->name,strlen(usermode->name));
	sqlEscapeProp(chanmask)
	sqlEscapeProp(comment)
	sqlEscapeProp(hostmask)
	sqlEscapeProp(machineid)
	sqlEscapeProp(setbyhost)
	sqlEscapeProp(setbynick)
	#undef sqlEscapeProp
//chanmask, comment,expires(int),hostmask,machineid,modeflags(int),profileid,setbyhost,setbynick,setbypid,setondate(int),usermodeid
	sprintf_s(query,len,"CALL SetUserMode(\"%s\",\"%s\",FROM_UNIXTIME(%d),\"%s\",\"%s\",%d,%d,\"%s\",\"%s\",%d)",chanmask,comment,usermode->expires,hostmask,machineid,usermode->modeflags,usermode->profileid,setbyhost,setbynick,usermode->setbypid);	
	if(mysql_query(server.conn,query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		exit(1);
	}
	clearResults(server.conn);	
	free((void *)query);
	}
	if(usermode->isGlobal == 0) {
		usermode->usermodeid = -((usermode->usermodeid * 2) * rand());
		server.usermodes_list.push_back(usermode);
		applyUserMode(usermode);
	}
}
void applyUserModes(Client *user) {
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *usermode;
	while(iterator != server.usermodes_list.end()) {
		usermode=*iterator;
		applyUserMode(usermode,user);
		iterator++;
	}
}
void applyUserMode(userMode *usermode, Client *user) {
	if(isGlobalExcempt(user))
		return;
	if(tolower(usermode->chanmask[0]) != 'x' || usermode->chanmask[1] != 0) {
		return;
	}
	if(usermodeMatches(user,usermode)) {
		if(usermode->modeflags & EModeFlags_Gag) {
			if(user->isGagged()) {
				return;
			}
			user->setGagged(true);
			char *nick,*reason;
			reason = (char *)&usermode->comment;
			user->getUserInfo(&nick,NULL,NULL,NULL);
			struct tm * timeinfo;
			timeinfo = localtime ( &usermode->expires);
			char timestr[128];
			if(usermode->expires != 0) {
				strftime(timestr,sizeof(timestr),"%m/%d/%Y %H:%M",timeinfo);
			} else {
				strcpy((char *)&timestr, "Indefinite");
			}
			user->sendToClient(":ADMIN!ADMIN@* PRIVMSG %s :Your chat privileges have been revoked from this service for violation of our acceptable use policy. The reason given is: %s. This will expire on: %s. You can continue to use the service to play games, but will not be allowed to chat with other users.",nick,reason,timestr);
		}
		if(usermode->modeflags & EModeFlags_Ban) {
			if(!user->checkQuit())
			user->quitUser("KILLED - KLINED");
		}
	}
}
bool isGlobalExcempt(Client *user) {
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *usermode;
	while(iterator != server.usermodes_list.end()) {
		usermode=*iterator;
		if(usermodeMatches(user, usermode)) {
			if(tolower(usermode->chanmask[0] == 'x') && usermode->chanmask[1] == 0) {
				if(usermode->modeflags & EModeFlags_BanExcempt) {
					return true;
				}
			}
		}
		iterator++;
	}
	return false;
}
bool isGlobalBanned(Client *user) {
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *usermode;
	while(iterator != server.usermodes_list.end()) {
		usermode=*iterator;
		if(usermodeMatches(user, usermode)) {
			if(tolower(usermode->chanmask[0]) == 'x' && usermode->chanmask[1] == 0) {
				if(usermode->modeflags & EModeFlags_Ban) {
					return true;
				}
			}
		}
		iterator++;
	}
	return false;
}
void applyUserMode(userMode *usermode) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	if(tolower(usermode->chanmask[0]) == 'x' && usermode->chanmask[1] == 0) { //global
		while(iterator != server.client_list.end()) {
			user=*iterator;
			applyUserMode(usermode,user);
			iterator++;
		}
	} else { //channel usermode
		std::list<Channel *> chanlist = server.chan_list;
		std::list<Channel *>::iterator iterator2=chanlist.begin();
		Channel *chan;
		while(iterator2 != chanlist.end()) {
			bool chanmatches = true;
			chan=*iterator2;
			char *name;
			name = chan->getName();
			if(stricmp(usermode->chanmask,name) != 0) {
				if(match(usermode->chanmask,name) != 0) {
					chanmatches = false;
				}
			}
			if(chanmatches) {
				applyChanUserModes(chan,usermode);
			}
			iterator2++;
		}
	}
}
void applyChanUserModes(Channel *chan,userMode *usermode) {
	if(chan->setup != 0xABCDEF02) return;
	std::list<chanClient *> clientList = chan->getList();
	std::list<chanClient *>::iterator iterator = clientList.begin();
	char *nick;
	char *servnick = "SERVER\0";
	if(usermode->hideNick == true || usermode->setbynick[0] == 0) {
		nick = servnick;
	} else {
		nick = (char *)&usermode->setbynick;
	}
	if(usermode->hostmask[0] != 0) {
		if(usermode->modeflags & EModeFlags_Ban) { //bans/invites/excempts don't set user specific modes
			chan->sendToChan(":%s!SERVER@* MODE %s +b *!*@%s",nick,chan->getName(),usermode->hostmask);
		}
		if(usermode->modeflags & EModeFlags_BanExcempt) {
			chan->sendToChan(":%s!SERVER@* MODE %s +E *!*@%s",nick,chan->getName(),usermode->hostmask);
		}
		if(usermode->modeflags & EModeFlags_Invited) {
			chan->sendToChan(":%s!SERVER@* MODE %s +I *!*@%s",nick,chan->getName(),usermode->hostmask);
		}
	}
	chanClient *user;
	while(iterator != clientList.end()) {
		user = *iterator;
		if(usermodeMatches(user->client,usermode)) {
			serversetChanModes(chan,user,usermode,true);
		}
		iterator++;
	}

}
void serversetChanModes(Channel *chan, chanClient *client, userMode *usermode,bool set) {
	bool invisible = client->invisible;
	char *nick,*setnick;
	char *servnick = "SERVER\0";
	if(usermode->hideNick == true || usermode->setbynick[0] == 0 || !set) {
		setnick = servnick;
	} else {
		setnick = (char *)&usermode->setbynick;
	}
	int modeflags = usermode->modeflags;
	client->client->getUserInfo(&nick,NULL,NULL,NULL);
	if(modeflags & EModeFlags_Voice) {
		if(client->voice != set) {
			if(invisible) {
				chan->invisibleSend(":%s!SERVER@* MODE %s %cv %s",setnick,chan->getName(),set==true?'+':'-',nick);
			} else {
				chan->sendToChan(":%s!SERVER@* MODE %s %cv %s",setnick,chan->getName(),set==true?'+':'-',nick);
			}
			client->voice = set;
		}
	}
	if(modeflags & EModeFlags_HalfOp || modeflags & EModeFlags_Op || modeflags & EModeFlags_Owner) {
		if(client->halfop != set || client->op != set || client->owner != set) {
			if(invisible) {
				chan->invisibleSend(":%s!SERVER@* MODE %s %co %s",setnick,chan->getName(),set==true?'+':'-',nick);
			} else {
				chan->sendToChan(":%s!SERVER@* MODE %s %co %s",setnick,chan->getName(),set==true?'+':'-',nick);
			}
		}
		if(modeflags & EModeFlags_HalfOp) {
			client->halfop = set;
		}
		if(modeflags & EModeFlags_Op) {
			client->op = set;
		}
		if(modeflags & EModeFlags_Owner) {
			client->owner = set;
		}
	}
	if(modeflags & EModeFlags_Gag) {
		client->gag = set;
	}
	if(modeflags & EModeFlags_Ban) {
		if(client->client->getRights() & OPERPRIVS_BANEXCEMPT || !set) return;
		if(invisible) {
			chan->invisibleSend(":%s!SERVER@* KICK %s %s :Banned",setnick,chan->getName(),nick);
		} else {
			chan->sendToChan(":%s!SERVER@* KICK %s %s :Banned",setnick,chan->getName(),nick);
		}
		chan->removeUser(client->client, false);
	}
}
bool usermodeMatches(Client *user, userMode *usermode) {
	char *host,*machineid;
	bool hostmatches = true,machineidmatches = true, pidmatches = true;
	user->getUserInfo(NULL,NULL,&host,&machineid);
	if(usermode->hostmask[0] != 0) {
		 if(stricmp(usermode->hostmask,host) != 0) {
			if(match(usermode->hostmask,host) != 0) {
				hostmatches = false;
			}
		} 
	}
	if(usermode->machineid[0] != 0) {
		if(stricmp(usermode->machineid,machineid) != 0) {
			if(match(usermode->machineid,machineid) != 0) {
				machineidmatches = false;
			}
		}
	}
	if(usermode->profileid != 0) {
		if(user->getProfileID() != usermode->profileid) {
			pidmatches = false;
		}
	}
	return hostmatches&&machineidmatches&&pidmatches;
}
void getModeStr(userMode *um, char *dst) {
	if(um->modeflags & EModeFlags_Voice) {
		strcat(dst,"v");
	}
	if(um->modeflags & EModeFlags_HalfOp) {
		strcat(dst,"h");
	}
	if(um->modeflags & EModeFlags_Op) {
		strcat(dst,"o");
	}
	if(um->modeflags & EModeFlags_Owner) {
		strcat(dst,"O");
	}
	if(um->modeflags & EModeFlags_Gag) {
		strcat(dst,"g");
	}
	if(um->modeflags & EModeFlags_Ban) {
		strcat(dst,"b");
	}
	if(um->modeflags & EModeFlags_Invited) {
		strcat(dst,"I");
	}
	if(um->modeflags & EModeFlags_BanExcempt) {
		strcat(dst,"E");
	}
}
void sendUserMode(Client *user,userMode *um) {
	char sendstr[512];
	memset(&sendstr,0,sizeof(sendstr));
	char *nick;
	user->getUserInfo(&nick,NULL,NULL,NULL);
	char modestr[32];
	char tempstr[128];
	memset(&modestr,0,sizeof(modestr));
	getModeStr(um,(char *)&modestr);
	sprintf(sendstr,":SERVER!SERVER@* PRIVMSG %s :LISTUSERMODE \\usermodeid\\%d\\chanmask\\%s\\modeflags\\%s",nick,um->usermodeid,um->chanmask,modestr);
	if(um->hostmask[0] != 0) {
		memset(&tempstr, 0, sizeof(tempstr));
		sprintf(tempstr, "\\hostmask\\%s",um->hostmask);
		strcat(sendstr, tempstr);
	}
	if(um->machineid[0] != 0) {
		memset(&tempstr, 0, sizeof(tempstr));
		sprintf(tempstr, "\\machineid\\%s",um->machineid);
		strcat(sendstr, tempstr);
	}
	if(um->profileid != 0) {
		memset(&tempstr, 0, sizeof(tempstr));
		sprintf(tempstr, "\\machineid\\%d",um->profileid);
		strcat(sendstr, tempstr);
	}

	sprintf(tempstr, "\\isGlobal\\%d",um->isGlobal);
	strcat(sendstr, tempstr);

	if(um->expires != 0) {
		struct tm * timeinfo;
		timeinfo = localtime ( &um->expires );
		memset(&tempstr, 0, sizeof(tempstr));
		char timestr[256];
		strftime(timestr,sizeof(timestr),"%m/%d/%Y %H:%M",timeinfo);
		sprintf(tempstr, "\\expires\\%s",timestr);
		strcat(sendstr, tempstr);
	}
	if(um->setbynick[0] != 0) {
		sprintf(tempstr, "\\setbynick\\%s",um->setbynick);
		strcat(sendstr, tempstr);
	}
	if(um->setbypid != 0) {
		sprintf(tempstr, "\\setbypid\\%d",um->setbypid);
		strcat(sendstr, tempstr);
	}
	if(um->setbyhost[0] != 0) {
		sprintf(tempstr, "\\setbyhost\\%s",um->setbyhost);
		strcat(sendstr, tempstr);
	}
	if(um->comment[0] != 0) {
		sprintf(tempstr, "\\comment\\%s",um->comment);
		strcat(sendstr, tempstr);
	}
	if(um->setondate != 0) {
		struct tm * timeinfo;
		timeinfo = localtime ( &um->setondate );
		memset(&tempstr, 0, sizeof(tempstr));
		char timestr[256];
		strftime(timestr,sizeof(timestr),"%m/%d/%Y %H:%M",timeinfo);
		sprintf(tempstr, "\\setondate\\%s",timestr);
		strcat(sendstr, tempstr);
	}
	user->sendToClient(sendstr);
}
int getUserChannelModes(Client *user, char *channame) { //name rather than chan pointer because the channel might not exist yet
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *usermode;
	bool chanMatches;
	int flags = 0;
	while(iterator != server.usermodes_list.end()) {
		usermode=*iterator;
		if(usermodeMatches(user, usermode)) {
			chanMatches = true;
			if(stricmp(usermode->chanmask,channame) != 0) {
				if(match(usermode->chanmask,channame) != 0) {
					chanMatches = false;
				}
			}
			if(chanMatches) {
				flags |= usermode->modeflags;
			}
		}
		iterator++;
	}
	return flags;
}
userMode *getUserMode(int usermodeid) {
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *usermode;
	while(iterator != server.usermodes_list.end()) {
		usermode=*iterator;
		if(usermode->usermodeid == usermodeid) {
			return usermode;
		}
		iterator++;
	}
	return NULL;
}
void removeUsermode(userMode *usermode, bool nosql) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	if(nosql || usermode->isGlobal == 0) {
		if(tolower(usermode->chanmask[0]) == 'x' && usermode->chanmask[1] == 0) {
			user=*iterator;
			if(usermodeMatches(user,usermode)) {
				if(usermode->modeflags & EModeFlags_Gag) {
					user->setGagged(false);
				}
			}
			iterator++;
		} else {
			std::list<Channel *>::iterator chanit=server.chan_list.begin();
			Channel *chan;
			while(chanit != server.chan_list.end()) {
				chan = *chanit;
				if(match(usermode->chanmask,chan->getName()) == 0) {
					removeChanUserModes(chan,usermode);
				}
				chanit++;
			}
	
		}
	}
	if(usermode->isGlobal == 1 && !nosql) {
		usermode->expires = 0;//stops infinite calls of this in the expiration checking
		char query[512];
		sprintf_s(query,sizeof(query),"CALL DelUserMode(%d)",usermode->usermodeid);
		if(mysql_query(server.conn,query)) {
		      fprintf(stderr, "%s\n", mysql_error(server.conn));
		      exit(1);
		}
		clearResults(server.conn);
	} else {
		server.usermodes_list.remove(usermode);
		free((void *)usermode);
	}
}
void removeChanUserModes(Channel *chan,userMode *usermode) {
	std::list<chanClient *> clientList = chan->getList();
	std::list<chanClient *>::iterator iterator = clientList.begin();
	if(usermode->hostmask[0] != 0) {
		if(usermode->modeflags & EModeFlags_Ban) { //bans/invites/excempts don't set user specific modes
			chan->sendToChan(":SERVER!SERVER@* MODE %s -b *!*@%s",chan->getName(),usermode->hostmask);
		}
		if(usermode->modeflags & EModeFlags_BanExcempt) {
			chan->sendToChan(":SERVER!SERVER@* MODE %s -E *!*@%s",chan->getName(),usermode->hostmask);
		}
		if(usermode->modeflags & EModeFlags_Invited) {
			chan->sendToChan(":SERVER!SERVER@* MODE %s -I *!*@%s",chan->getName(),usermode->hostmask);
		}
	}
	chanClient *user;
	while(iterator != clientList.end()) {
		user = *iterator;
		if(usermodeMatches(user->client,usermode)) {
			serversetChanModes(chan,user,usermode,false);
		}
		iterator++;
	}

}
userMode *findRemoveableUsermode(char *name, char *hostmask, int modeflags, bool wildcard) {
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *usermode;
	char *p = strchr(hostmask,'@');
	if(p != NULL) {
		hostmask = p+1;
	}
	while(iterator != server.usermodes_list.end()) {
		usermode=*iterator;
		if(stricmp(usermode->hostmask,hostmask) == 0) {
			if(stricmp(usermode->chanmask,name) == 0 || (wildcard && match(usermode->chanmask,name) == 0)) {
				if(usermode->modeflags == modeflags) {
					return usermode;
				}
			}
		}
		iterator++;
	}
	return NULL;
}
void addChanProp(Client *setter, char *target, char *modestr) {
	char data[256];
	char *nick, *host;
	int pid;
	chanProps *props;
	bool newProp = false;
	MYSQL_RES *res;
	MYSQL_ROW row;
	props = findChanProp(target);
	if(props == NULL) {
		if(~setter->getRights() & OPERPRIVS_GLOBALOWNER) {
			setter->send_numeric(482,"%s :You're not channel operator (Register new channel/Update wildcard props - OP_GLOBAL_OWNER)",target);
			return;
		}
		newProp = true;
		props = (chanProps *)malloc(sizeof(chanProps));;
		memset(props,0,sizeof(chanProps));
	} else { //exists, so check if they are oper
		if(~setter->getRights() & OPERPRIVS_GLOBALOWNER) {
			Channel *chan = find_chan(target);
			if(chan != NULL) {
				chanClient *user = chan->getUserInfo(setter);
				if(!user->owner) {
					setter->send_numeric(482, "%s :You're not channel operator (Set props - OWNER)",target);
					return;
				}
			} else {
				setter->send_numeric(403, "%s :No such channel",target);
				return;
			}
		
		}
	}
	if(props == NULL) return;
	if(find_param("topic", modestr, data, MAX_COMMENT)) {
		strcpy(props->topic,data);
	}
	if(find_param("entrymsg", modestr, data, MAX_COMMENT)) {
		strcpy(props->entrymsg,data);
	}
	if(find_param("comment", modestr, data, MAX_COMMENT)) {
		strcpy(props->comment,data);
	}
	if(find_param("onlyowner", modestr, data, MAX_NAME)) {
		props->onlyowner = atoi(data)==1?true:false;
	}
	if(find_param("limit", modestr, data, MAX_NAME)) {
		props->limit = atoi(data);
	}
	if(find_param("chankey", modestr, data, MAX_NAME)) {
		strcpy(props->chankey,data);
	}
	if(find_param("mode", modestr, data, MAX_NAME)) {
		strcpy(props->modes,data);
	}
	if(find_param("groupname", modestr, data, MAX_NAME)) {
		strcpy(props->groupname,data);
	}
	bool kickexisting = false;
	if(find_param("kickexisting", modestr, data, MAX_NAME)) {
		if(atoi(data) == 1) {
			kickexisting = true;
		}
	}
	if(find_param("expiressec", modestr, data, MAX_NAME)) {
		props->expires = atoi(data) + time(NULL);
	}
	pid = setter->getProfileID();
	setter->getUserInfo(&nick,NULL,&host,NULL);
	strcpy(props->setbynick,nick);
	strcpy(props->setbyhost, host);
	props->setbypid = pid;
	props->setondate = time(NULL);
	strcpy(props->chanmask,target);
	int qsize = sizeof(userMode) * 2;
	char *query = (char *)malloc(qsize);
	char topic[MAX_COMMENT];
	char entrymsg[MAX_COMMENT];
	char comment[MAX_COMMENT];
	char modes[MAX_NAME];
	char groupname[MAX_NAME];
	char chanmask[MAX_NAME];
	char chankey[MAX_NAME];
	char setbynick[MAX_NAME];
	char setbyhost[MAX_NAME];
	#define sqlEscapeProp(name) mysql_real_escape_string(server.conn,name,props->name,strlen(props->name));
	sqlEscapeProp(topic)
	sqlEscapeProp(entrymsg)
	sqlEscapeProp(comment)
	sqlEscapeProp(modes)
	sqlEscapeProp(groupname)
	sqlEscapeProp(chanmask)
	sqlEscapeProp(chankey)
	sqlEscapeProp(setbynick)
	sqlEscapeProp(setbyhost)
	#undef sqlEscapeProp
	
	sprintf_s(query,qsize,"CALL SetChanProps(\"%s\",\"%s\",\"%s\",\"%s\",FROM_UNIXTIME(%d),\"%s\",%d,\"%s\",%d,\"%s\",\"%s\",%d,\"%s\",%d)",chanmask,chankey,comment,entrymsg,props->expires,groupname,props->limit,modes,props->onlyowner,setbynick,setbyhost,props->setbypid,topic,kickexisting);
	if(mysql_query(server.conn,query)) {
      		fprintf(stderr, "%s\n", mysql_error(server.conn));
     		exit(1);
	}
	clearResults(server.conn);
	free((void *)query);
	if(newProp) {
		free((void *)props);
	}
}
void applyChanProps(chanProps *props, bool kickexisting) {
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	Channel *chan;
	if(strchr(props->modes,'z') != NULL && strchr(props->chanmask,'*') == NULL) {
		if(find_chan(props->chanmask) == NULL) {
			chan = new Channel(props->chanmask);
			chan->applyChanProps(props,false);
			return;
		}
	}
	while(iterator != server.chan_list.end()) {
		chan = *iterator;
		if(match(props->chanmask,(const char *)chan->getName()) == 0) {
			if(getClosestChanProp(chan->getName()) == props)
				chan->applyChanProps(props,kickexisting);
		}
		iterator++;
	}
}
chanProps *findChanProp(char *mask) {
	std::list<chanProps *>::iterator iterator=server.chanprops_list.begin();
	chanProps *props;
	while(iterator != server.chanprops_list.end()) {
		props = *iterator;
		if(stricmp(props->chanmask,mask) == 0) {
			return props;
		}
		iterator++;
	}
	return NULL;
}
chanProps *getClosestChanProp(char *channame, chanProps *skip) {
	chanProps *chanprop = findChanProp(channame);
	if(chanprop != NULL) return chanprop==skip?NULL:chanprop;
	std::list<chanProps *>::iterator iterator=server.chanprops_list.begin();
	chanProps *props = NULL;
	chanProps *retprops = NULL;
	while(iterator != server.chanprops_list.end()) {
		props = *iterator;
		if(match(props->chanmask,channame) == 0) {
			if(retprops == NULL || strlen(props->chanmask) > strlen(retprops->chanmask)) {
				if(props != skip)
					retprops = props;
			}
		}
		iterator++;
	}
	return retprops;
}
void deleteChanProp(chanProps *chanprop, bool resetchan) {
	std::list<Channel *>::iterator iterator=server.chan_list.begin();
	Channel *chan;
	while(iterator != server.chan_list.end()) {
		chan = *iterator;
		if(getClosestChanProp(chan->getName()) == chanprop) {
			if(resetchan) {
				chan->resetChannel(true);
				iterator=server.chan_list.begin(); //the channel list would have been changed
				continue;
			} else {
				chanProps *props = getClosestChanProp(chan->getName(),chanprop);
				chan->applyChanProps(props, false); //if its null its handled in here, where it deletes the chan modes
			}
		}
		iterator++;
	}
	server.chanprops_list.remove(chanprop);
	free((void *)chanprop);
	return;
}
void deleteChanProp(char *chanmask, bool resetchan) {
	char query[1024];
	char name[MAX_NAME];
	mysql_real_escape_string(server.conn,name,chanmask,strlen(chanmask));
	sprintf_s(query,sizeof(query),"CALL DelChanProps(\"%s\",%d)",name,resetchan);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		exit(1);


	}
	clearResults(server.conn);
}
void checkExpiry() {
	std::list<chanProps *>::iterator iterator=server.chanprops_list.begin();
	chanProps *props = NULL;
	chanProps *retprops = NULL;
	while(iterator != server.chanprops_list.end()) {
		props = *iterator;
		if(props->expires != 0) {
			if(props->expires <= time(NULL)) {
				props->expires = 0;
				deleteChanProp(props->chanmask,true);
//				iterator=server.chanprops_list.begin();
				continue;
			}
		}
		iterator++;

	}
	std::list<userMode *>::iterator iterator2=server.usermodes_list.begin();
	userMode *usermode;
	while(iterator2 != server.usermodes_list.end()) {
		usermode=*iterator2;
		if(usermode->expires != 0) {
			if(usermode->expires <= time(NULL)) {
				removeUsermode(usermode);
//				iterator2=server.usermodes_list.begin();
				continue;
			}
		}
		iterator2++;
	}
	std::list<whowasInfo *>::iterator iterator3=server.whowas_list.begin();
	whowasInfo *info;
	while(iterator3 != server.whowas_list.end()) {
		info = *iterator3;
		if(info->quittime+WHOWAS_TIMEOUT<time(NULL)) {
			server.whowas_list.remove(info);
			iterator3=server.whowas_list.begin();
			continue;
		}
		iterator3++;
	}
}
void sendChanProps(Client *who, chanProps *prop) {
	char sendstr[512];
	memset(&sendstr,0,sizeof(sendstr));
	char *nick;
	who->getUserInfo(&nick,NULL,NULL,NULL);
	char tempstr[128];
	
	sprintf(sendstr,":SERVER!SERVER@* PRIVMSG %s :LISTCHANPROPS \\chanmask\\%s\\onlyowner\\%d",nick,prop->chanmask,prop->onlyowner);
	if(prop->modes[0] != 0) {
		sprintf(tempstr, "\\mode\\%s",prop->modes);
		strcat(sendstr, tempstr);
	}
	if(prop->topic[0] != 0) {
		sprintf(tempstr, "\\topic\\%s",prop->topic);
		strcat(sendstr, tempstr);
	}
	if(prop->entrymsg[0] != 0) {
		sprintf(tempstr, "\\entrymsg\\%s",prop->entrymsg);
		strcat(sendstr, tempstr);
	}
	if(prop->groupname[0] != 0) {
		sprintf(tempstr, "\\groupname\\%s",prop->groupname);
		strcat(sendstr, tempstr);
	}
	if(prop->limit != 0) {
		sprintf(tempstr, "\\limit\\%d",prop->limit);
		strcat(sendstr, tempstr);
	}
	if(prop->chankey[0] != 0) {
		sprintf(tempstr, "\\chankey\\%s",prop->chankey);
		strcat(sendstr, tempstr);
	}
	if(prop->expires != 0) {
		struct tm * timeinfo;
		timeinfo = localtime ( &prop->expires );
		memset(&tempstr, 0, sizeof(tempstr));
		char timestr[256];
		strftime(timestr,sizeof(timestr),"%m/%d/%Y %H:%M",timeinfo);
		sprintf(tempstr, "\\expires\\%s",timestr);
		strcat(sendstr, tempstr);
	}
	if(prop->setbynick[0] != 0) {
		sprintf(tempstr, "\\setbynick\\%s",prop->setbynick);
		strcat(sendstr, tempstr);
	}
	if(prop->setbypid != 0) {
		sprintf(tempstr, "\\setbypid\\%d",prop->setbypid);
		strcat(sendstr, tempstr);
	}
	if(prop->setbyhost != 0) {
		sprintf(tempstr, "\\setbyhost\\%s",prop->setbyhost);
		strcat(sendstr, tempstr);
	}
	if(prop->comment[0] != 0) {
		sprintf(tempstr, "\\comment\\%s",prop->comment);
		strcat(sendstr, tempstr);
	}
	if(prop->setondate != 0) {
		struct tm * timeinfo;
		timeinfo = localtime ( &prop->setondate );
		memset(&tempstr, 0, sizeof(tempstr));
		char timestr[256];
		strftime(timestr,sizeof(timestr),"%m/%d/%Y %H:%M",timeinfo);
		sprintf(tempstr, "\\setondate\\%s",timestr);
		strcat(sendstr, tempstr);
	}
	who->sendToClient(sendstr);
}
	//std::list <whowasInfo *> whowas_list
void addWhowas(Client *user) { //at the user to the whowas list before they quit
	whowasInfo *info = (whowasInfo *)malloc(sizeof(whowasInfo));
	if(info != NULL) {
		memset(info, 0, sizeof(whowasInfo));
		char *nick, *username, *host, *realname;
		user->getUserInfo(&nick,&username,&host,&realname);
		strcpy(info->name,nick);
		strcpy(info->username,username);
		strcpy(info->host,host);
		strcpy(info->realname,realname);
		info->quittime = time(NULL);
		server.whowas_list.push_back(info);
	}
}
bool getKey(std::list<customKey *> userKeys, char *name, char *dst,int len) {
	return getKey(&userKeys,name,dst,len);
}
bool getKey(std::list<customKey *> *userKeys, char *name, char *dst,int len) {
	std::list<customKey *>::iterator iterator = userKeys->begin();
	customKey *key;
	while(iterator != userKeys->end()) {
		key = *iterator;
		if(stricmp(key->name,name) == 0) {
			if(dst != NULL)
				strncpy(dst,key->value,strlen(key->value)%len);
			return true;
		}
		iterator++;
	}
	if(dst != NULL)
		dst[0] = 0;
	return false;
}
gameInfo *findGame(char *name) {
	int i;
	if(name == NULL) return NULL;
	for(i=0;i<server.num_games;i++) {
		if(server.games[i].name == NULL) continue;
		if(strcasecmp(server.games[i].name,name) == 0) {
			return &server.games[i];
		}
	}
	return NULL;
}
operInfo *getOperInfo(int profileid) {
	operInfo *info;
	std::list<operInfo *>::iterator iterator = server.oper_list.begin();
	while(iterator != server.oper_list.end()) {
		info = *iterator;
		if(info->profileid == profileid) {
			return info;
		}
		iterator++;
	}
	return NULL;
}
void sendAllChanNames(Client *target) { //for when you are off -q, etc
	std::list<Channel *>::iterator channels = server.chan_list.begin();
	Channel *chan;
	while(channels != server.chan_list.end()) {
		chan = *channels;
		if(chan->userOn(target)) {
			chan->sendNames(target);
		}
		channels++;
	}
}
void sendToAllWithMode(uint32_t mode, char *str,...) {
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	std::list<Client *> list = server.client_list;
	std::list<Client *>::iterator iterator;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, str );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),str,args);
	va_end(args);
	iterator=list.begin();
	Client *c;
	while(iterator != list.end()) {
		c=*iterator;
		if(c->getModeFlags() & mode) {
			c->sendToClient(sbuff);
		}
		iterator++;
	}
}
void addChannelInvite(Client *user,Channel *chan) {
	inviteInfo *invite = (inviteInfo *)malloc(sizeof(inviteInfo));
	invite->invited = user;
	invite->chan = chan;
	server.invite_list.push_back(invite);
}
bool isUserInvited(Client *user, Channel *chan) {
	std::list<inviteInfo *>::iterator iterator = server.invite_list.begin();
	inviteInfo *info;
	while(iterator != server.invite_list.end()) {
		info = *iterator;
		if(info->invited == user && info->chan == chan) {
			return true;
		}
		iterator++;
	}
	return false;
}
void removeChannelInvite(Client *user, Channel *chan) {
	std::list<inviteInfo *>::iterator iterator = server.invite_list.begin();
	inviteInfo *info;
	while(iterator != server.invite_list.end()) {
		info = *iterator;
		if((user == NULL || info->invited == user) && (chan == NULL || info->chan == chan)) {
			server.invite_list.erase(iterator);
			free((void *)info);
			iterator = server.invite_list.begin();
		}
		iterator++;
	}
}
void resetByProfileID(int profileid, char *reason) {
	std::list<Client *>::iterator iterator;
	iterator=server.client_list.begin();
	Client *user;
	while(iterator != server.client_list.end()) {
		user = *iterator;
		if(user->getProfileID() == profileid) {
			user->quitUser(reason);
		}
		iterator++;

	}	
}
void addOper(int profileid, uint32_t rightsmask) {
	operInfo *oper;
	oper = getOperInfo(profileid);
	if(oper == NULL) {
		oper = (operInfo *)malloc(sizeof(operInfo));
		oper->profileid = profileid;
		oper->rightsmask = rightsmask;
		server.oper_list.push_back(oper);
	} else {
		oper->rightsmask = rightsmask;
		resetByProfileID(profileid,"Oper privs reset");
	}
}
void delOper(int profileid) {
	operInfo *oper;
	oper = getOperInfo(profileid);
	if(oper == NULL) return;
	resetByProfileID(profileid,"Oper privs revoked");
	server.oper_list.remove(oper);
	free((void *)oper);
}
bool isClientAllowed(Client *user, char *channame) {
	bool foundone = false;
	std::list<chanGameClient *>::iterator it = server.chanclient_list.begin();
	chanGameClient *client;
	int gameid = user->getGameID();
	while(it != server.chanclient_list.end()) {
		client = *it;
		if(match(client->chanmask,channame) == 0) {
			if(gameid != 0 && client->gameid == -1) {
				return true;
			}
			if(gameid == client->gameid) {
				return true;
			}
			foundone = true;
		}
		it++;
	}
	return foundone==false;
}
chanGameClient *findGameClient(char *chanmask, int gameid) {
	std::list<chanGameClient *>::iterator it = server.chanclient_list.begin();
	chanGameClient *client;
	while(it != server.chanclient_list.end()) {
		client = *it;
		if(strcasecmp(client->chanmask,chanmask) == 0) {
			if(client->gameid == gameid) {
				return client;
			}
		}
		it++;
	}
	return NULL;
}
bool deleteGameClient(char *chanmask, int gameid) {
	chanGameClient *client = findGameClient(chanmask,gameid);
	if(client != NULL) {
		server.chanclient_list.remove(client);
		free((void *)client);
	}
	return false;
}
Client *findClientBySocket(int sd) {
	std::list<Client *>::iterator it = server.client_list.begin();
	Client *client;
	while(it != server.client_list.end()) {
		client = *it;
		if(client->getSocket() == sd) {
			return client;
		}
		it++;
	}
	return NULL;
}
int numUsersByIP(uint32_t ip) {
	int i=0;
	std::list<Client *>::iterator it = server.client_list.begin();
	Client *client;
	while(it != server.client_list.end()) {
		client = *it;
		if(client->getIP() == ip) {
			i++;
		}
		it++;
	}
	return i;	
}
