#include "channel.h"
#include "structs.h"
extern peerchatServer server;
Channel::Channel(char *name) {
	memset(&this->name,0,sizeof(name));
	moderated=false;
	inviteonly=false;
	priv = false;
	secret = false;
	limit=0;
	stayopen=false;
	nooutsidemsgs = false;
	topic_protect = false;
	mode_ops_obey_channel_limit = false;
	auditorium=0;
	memset(&key,0,sizeof(key));
	memset(&topic,0,sizeof(topic));
	memset(&topicsetter,0,sizeof(topicsetter));
	memset(&groupname,0, sizeof(groupname));
	memset(&entrymsg,0,sizeof(entrymsg));
	topictime=0;
	allops = 0;
	registered = 0;
	onlyowner = 0;
	createtime=time(NULL);
	strncpy(this->name,name,(strlen(name)+1)%sizeof(this->name));
	setup = 0xABCDEF02;
	server.chan_list.push_back(this);
}
Channel::~Channel() {
	//deleteChannel(this);
	std::list<customKey *>::iterator iterator = chanKeys.begin();
	customKey *key;
	clearnonGlobalModes();
	while(iterator != chanKeys.end()) {
		key = *iterator;
		free((void *)key); //dw about removing from list since its going to be freed anyways
		iterator++;
	}
	std::list<chanClient *>::iterator iterator2 = user_list.begin();
	chanClient *user;
	while(iterator2 != user_list.end()) {
		user = *iterator2;
		std::list<customKey *>::iterator it2 = user->userKeys->begin();
		while(it2 != user->userKeys->end()) {
			key = *it2;
			free((void *)key);
			it2++;
		}
		delete user->userKeys;
		free((void *)user); //dw about removing from list since its going to be freed anyways
		iterator2++;
	}
	setup = 666;
	removeChannelInvite(NULL,this);
}
char *Channel::getName() {
	return (char *)(&(this->name));
}
void Channel::addUser(chanClient *user) {
	chanClient *chan_client=(chanClient *)malloc(sizeof(chanClient));
	char *unick;
	if(user == NULL) return;
	if(user->client == NULL) return;
	if(chan_client == NULL) {
		if(user->client != NULL) {
			user->client->send_numeric(473,"%s :Cannot join channel (Failed to allocate memory for chanClient)",getName());
		}
		return;
	}
	user_list.push_back(chan_client);
	user->client->getUserInfo(&unick,NULL,NULL,NULL);
	memcpy(chan_client,user,sizeof(chanClient));
	removeChannelInvite(user->client,this);
	chan_client->userKeys = new std::list<customKey *>();
	sendToAllWithMode((int)EUserMode_ShowJoins,":SERVER!SERVER@* NOTICE :%s has joined %s",unick,getName());
	if(this->auditorium != 2) {
		sendMessage(chan_client->invisible,false,chan_client->client,"JOIN %s",getName());
		if(chan_client->invisible) {
			invisibleSend(":SERVER!SERVER@* NOTICE %s :INVISIBLE USER %s HAS JOINED",getName(),unick);
		}
		if(this->getNumUsers(true) > 1) {
			if(chan_client->invisible) {
				if(chan_client->halfop||chan_client->op||chan_client->owner) {
					invisibleSend(":SERVER!SERVER@* MODE %s +o %s",getName(),unick);
				}
				if(chan_client->voice) {
					invisibleSend(":SERVER!SERVER@* MODE %s +v %s",getName(),unick);
				}
			} else {
				if(chan_client->halfop||chan_client->op||chan_client->owner) {
					sendToChan(":SERVER!SERVER@* MODE %s +o %s",getName(),unick);
				}
				if(chan_client->voice) {
					sendToChan(":SERVER!SERVER@* MODE %s +v %s",getName(),unick);
				}
			}
		}
	} else { //auditorium
		chan_client->client->messageSend(chan_client->client,"JOIN %s",getName());
	}
	sendNames(user->client);
	sendTopic(user->client);
	if(entrymsg[0] != 0) {
		user->client->sendToClient(":SERVER!SERVER@* NOTICE %s :%s",getName(),entrymsg);
	}
	return;
}
void Channel::removeUser(Client *user, bool sendpart) {
	removeUser(user,sendpart,"");
}
void Channel::removeUser(Client *user, bool sendpart, char *reason, bool closeChan) {
	std::list<chanClient *>::iterator it = user_list.begin();
	char *nullreason = "\0";
	if(reason == NULL) {
		reason = nullreason;
	}
	char *unick;
	user->getUserInfo(&unick,NULL,NULL,NULL);
	sendToAllWithMode((int)EUserMode_ShowJoins,":SERVER!SERVER@* NOTICE :%s has parted %s",unick,getName());
	while(it != user_list.end()) {
		chanClient *c=(chanClient *)*it;
		if(c->client == user) {
			if(sendpart) {
				if(this->auditorium != 2) {
					if(this->auditorium == 1) {
						if(c->voice || c->halfop || c->op || c->owner)
							sendMessage(c->invisible,false,user,"PART %s :%s",getName(),reason);
					} else {
						sendMessage(c->invisible,false,user,"PART %s :%s",getName(),reason);
					}
				} else {
					user->messageSend(user,"PART %s :%s",getName(),reason);
				}
			}
			customKey *key;
			std::list<customKey *>::iterator it2 = c->userKeys->begin();
			while(it2 != c->userKeys->end()) {
				key = *it2;
				free((void *)key);
				it2++;
			}
			delete c->userKeys;
			free(c);
			user_list.erase(it);
			break;
		}
		it++;
	}
	if(getNumUsers(true) < 1 && !stayopen && closeChan) {
		deleteChannel(this);
	}
}
bool Channel::userOn(Client *user) {
	return getUserInfo(user)==NULL?false:true;
}
chanClient *Channel::getUserInfo(Client *user) {
	std::list<chanClient *>::iterator it = user_list.begin();
	while(it != user_list.end()) {
		chanClient *c=(chanClient *)*it;
		if(c->client == user) {
			return c;
		}
		it++;
	}
	return NULL;
}
void Channel::sendToChanExcept(Client *exception, char *fmt, ...) {
	std::list<chanClient *>::iterator it = user_list.begin();
	chanClient *info;
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	while(it != user_list.end()) {
		info=*it;
		if(info->client != exception) {
			info->client->sendToClient(sbuff);
		}
		it++;
	}	
}
void Channel::invisibleSend(char *fmt,...) { //send to those who can see invisibles
	std::list<chanClient *>::iterator it = user_list.begin();
	chanClient *info;
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	while(it != user_list.end()) {
		info=*it;
		if(info->client->getRights()&OPERPRIVS_INVISIBLE) {
			info->client->sendToClient(sbuff);
		}
		it++;
	}	
}
void Channel::noninvisibleSend(char *fmt,...) {
	std::list<chanClient *>::iterator it = user_list.begin();
	chanClient *info;
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	while(it != user_list.end()) {
		info=*it;
		if(~info->client->getRights()&OPERPRIVS_INVISIBLE) {
			info->client->sendToClient(sbuff);
		}
		it++;
	}	
}
void Channel::sendToChan(char *fmt,...) {
	std::list<chanClient *>::iterator it = user_list.begin();
	chanClient *info;
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	while(it != user_list.end()) {
		info=*it;
		info->client->sendToClient(sbuff);
		it++;
	}	
}
void Channel::sendMessage(bool invisible, bool skip_sender, Client *sender, char *fmt, ...) {
	std::list<chanClient *>::iterator it = user_list.begin();
	chanClient *info;
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	while(it != user_list.end()) {
		info=*it;
		if(!invisible || info->client->getRights()&OPERPRIVS_INVISIBLE) {
			if(info->client != sender || !skip_sender)
				info->client->messageSend(sender,sbuff);
		}
		it++;
	}
}
std::list<chanClient *> Channel::getList() {
	return user_list;
}
int Channel::getNumUsers(bool showinvisible) {
	std::list<chanClient *>::iterator it = user_list.begin();
	int num=0;
	chanClient *client;
	while(it != user_list.end()) {
		client = *it;
		if(client->invisible) {
			if(!showinvisible) {
				num--;
			}
		}
		it++;
		num++;
	}
	return num;
}
void Channel::sendNames(Client *who) {
	chanClient *info;
	std::list<chanClient *>::iterator it = user_list.begin();
	int num=0;
	char *nick;
	int len = 1024;
	char *sbuff = (char *)malloc(len);
	sprintf_s(sbuff,len,"= %s :",getName());
	while(it != user_list.end()) {
		info=*it;
		if(info->invisible==true&&(~who->getRights()&OPERPRIVS_INVISIBLE)) {
			it++;
			continue;
		}
		if(this->auditorium == 2) {
			it++;
			continue;
		} else if(this->auditorium == 1) {
			if(info->halfop == false && info->op == false && info->owner == false && info->voice == false) {
			it++;
			continue;
			}
		}
		if(num != 0)
			strcat(sbuff," ");
		if(info->halfop||info->op||info->owner) {
			strcat(sbuff,"@");
		} else if(info->voice) {
			strcat(sbuff,"+");
		}
		info->client->getUserInfo(&nick,NULL,NULL,NULL);
		strcat(sbuff,nick);
		if(strlen(sbuff) > (len/2)) {
			len*=2;
			sbuff = (char *)realloc(sbuff,len);
		}
		it++;
		num++;
	}
	who->send_numeric(353,sbuff);
	free((void *)sbuff);
	who->send_numeric(366,"%s :End of NAMES list",getName());
}
void Channel::sendModes(Client *who) {
	char modestr[64];
	memset(&modestr,0,sizeof(modestr));
	strcat(modestr,"+");
	if(this->topic_protect) {
		strcat(modestr,"t");
	}
	if(this->nooutsidemsgs) {
		strcat(modestr,"n");
	}
	if(this->auditorium==1) {
		strcat(modestr,"q");
	} else if(this->auditorium == 2) {
		strcat(modestr,"u");
	}
	if(this->moderated) {
		strcat(modestr,"m");
	}
	if(this->priv) {
		strcat(modestr,"p");
	}
	if(this->secret) {
		strcat(modestr,"s");
	}
	if(this->inviteonly) {
		strcat(modestr,"i");
	}
	if(this->limit != 0) {
		strcat(modestr,"l");
	}
	if(this->key[0] != 0) {
		strcat(modestr,"k");
	}
	if(this->mode_ops_obey_channel_limit != 0) {
		strcat(modestr,"e");
	}
	if(this->userOn(who) || who->getRights() & OPERPRIVS_OPEROVERRIDE) {
		if(this->limit != 0) {
			char str[64];
			sprintf(str," %d",this->limit);
			strcat(modestr,str);
		}
		if(this->key[0] != 0) {
			char str[64];
			sprintf(str," %s",this->key);
			strcat(modestr,str);
		}
	}
	who->send_numeric(324,"%s %s",getName(),modestr);

}
void Channel::sendNotEnoughPrivs(ENotEnoughParams type, Client *who, const char *comment) {
	switch(type) {
	case ENotEnough_OP:
		who->send_numeric(482,"%s :You're not channel operator(%s - FULL OP)",getName(),comment);
		break;
	case ENotEnough_HALFOP:
		who->send_numeric(482,"%s :You're not channel operator(%s - HALF OP)",getName(),comment);
		break;
	case ENotEnough_OWNER:
		who->send_numeric(482,"%s :You're not channel operator(%s - OWNER)",getName(),comment);
		break;
	}
}
void Channel::setModes(Client *setter, char *modestr) {
	chanClient *info;
	chanClient tinfo;
	bool set=true;
	char tbuff[64];
	int modep=0;
	bool canVoice;
	char oldMode = this->auditorium;
	char *p=strchr(modestr,' ');
	if(p != NULL) *p++ = 0;
	memset(&tinfo,0,sizeof(tinfo));
	bool operoverride=false;
	char *tbuffptr;
	if(setter->getRights()&OPERPRIVS_OPEROVERRIDE) operoverride=true;
	//if(setter->getRights()&OPERPRIVS_GLOBALOWNER)  operoverride=true;
	info = getUserInfo(setter);
	if(info == NULL) info = &tinfo;
	if(info->owner) info->op = true;
	if(info->op) info->halfop = true;
	for(int i=0;i<strlen(modestr);i++) {
		switch(modestr[i]) {
		case '-':
			set = false;
			break;
		case '+':
			set = true;
			break;
		case 'e':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(this->mode_ops_obey_channel_limit == set) break;
				this->mode_ops_obey_channel_limit = set;
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+e":"-e");
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'm':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(this->moderated == set) break;
				this->moderated = set;
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+m":"-m");
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'n':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(this->nooutsidemsgs == set) break;
				this->nooutsidemsgs = set;
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+n":"-n");
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'i':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(this->inviteonly == set) break;
				this->inviteonly = set;
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+i":"-i");
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'l': //set channel limit
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
			int len = 0;
			if(set) {
				if(p == NULL) {
					info->client->send_numeric(461, "%c :Not enough parameters",modestr[i]);
					return;
				}
				find_nth(p,modep++,tbuff,sizeof(tbuff));
				if(strlen(tbuff)<1) return;
				len = atoi(tbuff);
				if(len < 1) return;
			}
			if(set) {
				sendMessage(false,false,setter,"MODE %s +l %d",getName(),len);
			} else {
				sendMessage(false,false,setter,"MODE %s -l",getName());
			}
			this->limit = len;
			} else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'k': //set channel key
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
			if(set) {
				if(p == NULL) {
					info->client->send_numeric(461, "%c :Not enough parameters",modestr[i]);
					return;
				}
				find_nth(p,modep++,tbuff,sizeof(tbuff));
				if(strlen(tbuff)<1) return;
				sendMessage(false,false,setter,"MODE %s +k %s",getName(),tbuff);
				strcpy(key,tbuff);
			} else {
				sendMessage(false,false,setter,"MODE %s -k",getName());
				memset(key,0,sizeof(key));
			}
			} else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 't':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(this->topic_protect == set) break;
				this->topic_protect = set;
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+t":"-t");
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'p':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(this->secret && set) {
					this->secret = false;
					sendMessage(false,false,setter,"MODE %s -s",getName());
				}
				if(this->priv == set) break;
				this->priv = set;
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+p":"-p");
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 's':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(this->priv && set) {
					this->priv = false;
					sendMessage(false,false,setter,"MODE %s -p",getName());
				}
				if(this->secret == set) break;
				this->secret = set;
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+s":"-s");
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'q':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(set && this->auditorium == 2) {
					sendMessage(false,false,setter,"MODE %s %s",getName(),!set?"+u":"-u");	
				}
				if((set && this->auditorium == 1) || (!set && this->auditorium != 1)) break;
				this->auditorium = (set==true?1:0);
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+q":"-q");
				auditoriumUpdate(oldMode);
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'u':
			if(info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				if(set && this->auditorium == 1) {
					sendMessage(false,false,setter,"MODE %s %s",getName(),!set?"+q":"-q");	
				}
				if((set && this->auditorium == 2) || (!set && this->auditorium != 2)) break;
				this->auditorium = (set==true?2:0);
				sendMessage(false,false,setter,"MODE %s %s",getName(),set?"+u":"-u");
				auditoriumUpdate(oldMode);
			}
			else sendNotEnoughPrivs(ENotEnough_HALFOP,setter,set?"set mode":"remove mode");
			break;
		case 'o': //op someone
			if(info->owner || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
			chanClient *tclient;
			Client *opclient;
			if(p == NULL) {
				info->client->send_numeric(461, "%c :Not enough parameters",modestr[i]);
				return;
			}
			find_nth(p,modep++,tbuff,sizeof(tbuff));
			if(strlen(tbuff)<1) return;
			opclient = find_user(tbuff);

			if(opclient != NULL) {
				tclient = this->getUserInfo(opclient);
				if(tclient != NULL) {
					if(tclient->op == set) break;
					sendMessage(false,false,setter,"MODE %s %s %s",getName(),set?"+o":"-o",tbuff);
					if(set == false) tclient->owner = set;
					tclient->op=set;
					tclient->halfop = set;
				} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
				}
			} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
				}
			} else {
				sendNotEnoughPrivs(ENotEnough_OWNER,setter,"change ops");
			}
			break;
		case 'v': //voice someone
			//checking isn't instant because you could be devoicing yourself
			chanClient *tclient;
			Client *opclient;
			if(p == NULL) {
				info->client->send_numeric(461, "%c :Not enough parameters",modestr[i]);
				return;
			}
			canVoice = info->halfop || info->op || info->owner || operoverride && (this->onlyowner != 1 || info->owner);
			find_nth(p,modep++,tbuff,sizeof(tbuff));
			if(strlen(tbuff)<1) return;
			opclient = find_user(tbuff);
			if(opclient != NULL) {
				tclient = this->getUserInfo(opclient);
				if(tclient != NULL) {
					if(!canVoice && (tclient->client != setter || set == true)) {
						sendNotEnoughPrivs(ENotEnough_HALFOP,setter,"change voice");
						break;
					}
					if(tclient->voice == set) break;
					sendMessage(false,false,setter,"MODE %s %s %s",getName(),set?"+v":"-v",tbuff);
					tclient->voice=set;
				} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
				}
			} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
			}
			break;
		case 'h': //halfop someone
			if(info->op || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
			chanClient *tclient;
			Client *opclient;
			if(p == NULL) {
				info->client->send_numeric(461, "%c :Not enough parameters",modestr[i]);
				return;
			}
			find_nth(p,modep++,tbuff,sizeof(tbuff));
			if(strlen(tbuff)<1) return;
			opclient = find_user(tbuff);
			if(opclient != NULL) {
				tclient = this->getUserInfo(opclient);
				if(tclient != NULL) {
					if(tclient->halfop == set) break;
					if(tclient->op == true || tclient->owner == true && info->owner == false) break;
						sendMessage(false,false,setter,"MODE %s %s %s",getName(),set?"+o":"-o",tbuff);
						tclient->halfop = set;
				} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
				}
			} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
				}
			} else {
				sendNotEnoughPrivs(ENotEnough_OP,setter,"set halfop");
			}
			break;
		case 'O': //owner someone
			if(info->owner || operoverride && !(this->onlyowner == 1 && info->owner == false)) {
				chanClient *tclient;
				Client *opclient;
				if(p == NULL) {
					info->client->send_numeric(461, "%c :Not enough parameters",modestr[i]);
					return;
				}
				find_nth(p,modep++,tbuff,sizeof(tbuff));
				if(strlen(tbuff)<1) return;
				opclient = find_user(tbuff);
			if(opclient != NULL) {
				tclient = this->getUserInfo(opclient);
				if(tclient != NULL) {
					if(tclient->owner == set) break;
					sendMessage(false,false,setter,"MODE %s %s %s",getName(),set?"+o":"-o",tbuff);
					tclient->owner = set;
					tclient->op = set;
					tclient->halfop = set;
				} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
				}
			} else {
					setter->send_numeric(401,"%s :No such nick/channel",tbuff);
					break;
				}
			} else {
				sendNotEnoughPrivs(ENotEnough_OWNER,setter,"change owner");
			}
			break;
		case 'b': //ban/unban someone
			if((info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) || p == NULL) {
				if(p == NULL) { //list bans
					std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
					userMode *um;
					char *nick;
					info->client->getUserInfo(&nick,NULL,NULL,NULL);
					while(iterator != server.usermodes_list.end()) {
						um=*iterator;
							if(match(um->chanmask,getName()) == 0) {
								if(um->hostmask[0] != 0 && um->modeflags & EModeFlags_Ban)
									info->client->send_numeric(367,"%s *!*@%s %s@%s %d",getName(), um->hostmask, um->setbynick[0] == 0?"SERVER":um->setbynick,um->setbyhost[0] == 0||info->halfop==false?"*":um->setbyhost,um->setondate);
							}
						iterator++;
					}
					info->client->send_numeric(368,"%s :End of channel ban list",getName());
					break;
				}
				find_nth(p,modep++,tbuff,sizeof(tbuff));
				if(strlen(tbuff)<1) return;
				char setstr[128];
				memset(&setstr,0, sizeof(setstr));
				tbuffptr = (char *)&tbuff;
				if(strchr(tbuffptr,'@') != NULL) tbuffptr = strchr(tbuffptr,'@')+1;
				sprintf(setstr,"\\hostmask\\%s\\modeflags\\b\\comment\\Banned by MODE command (Persist)",tbuffptr);
				if(set) {
					addUserMode(info->client,this->getName(),setstr);
				} else {
					userMode *usermode = findRemoveableUsermode(getName(),tbuff,EModeFlags_Ban,setter->getRights()&OPERPRIVS_GLOBALOWNER);
					if(usermode != NULL) {
						removeUsermode(usermode);
					}
				}
				//tbuff = ban name
			} else {
				sendNotEnoughPrivs(ENotEnough_HALFOP,setter,"set mode");
			}
			break;
		case 'I': //invite/uninvite someone
			if((info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) || p == NULL) {
				if(p == NULL) { //list bans
					std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
					userMode *um;
					char *nick;
					info->client->getUserInfo(&nick,NULL,NULL,NULL);
					while(iterator != server.usermodes_list.end()) {
						um=*iterator;
							if(match(um->chanmask,getName())) {
								if(um->hostmask[0] != 0 && um->modeflags & EModeFlags_Invited)
									info->client->send_numeric(367,"%s *!*@%s %s@%s %d",getName(), um->hostmask, um->setbynick[0] == 0?"SERVER":um->setbynick,um->setbyhost[0] == 0||info->halfop==false?"*":um->setbyhost,um->setondate);
							}
						iterator++;
					}
					info->client->send_numeric(368,"%s :End of channel invite list",getName());
					break;
				}
				find_nth(p,modep++,tbuff,sizeof(tbuff));
				if(strlen(tbuff)<1) return;
				char setstr[128];
				memset(&setstr,0, sizeof(setstr));
				tbuffptr = (char *)&tbuff;
				if(strchr(tbuffptr,'@') != NULL) tbuffptr = strchr(tbuffptr,'@')+1;
				sprintf(setstr,"\\hostmask\\%s\\modeflags\\I\\comment\\Invited by MODE command (Persist)",tbuffptr);
				if(set) {
					addUserMode(info->client,this->getName(),setstr);
				} else {
					userMode *usermode = findRemoveableUsermode(getName(),tbuff,EModeFlags_Invited,setter->getRights()&OPERPRIVS_GLOBALOWNER);
					if(usermode != NULL) {
						removeUsermode(usermode);
					}
				}
				//tbuff = ban name
			} else {
				sendNotEnoughPrivs(ENotEnough_HALFOP,setter,"set mode");
			}
			break;
		case 'E': //excempt/unexcempt someone
			if((info->halfop || operoverride && !(this->onlyowner == 1 && info->owner == false)) || p == NULL) {
				if(p == NULL) { //list bans
					std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
					userMode *um;
					char *nick;
					info->client->getUserInfo(&nick,NULL,NULL,NULL);
					while(iterator != server.usermodes_list.end()) {
						um=*iterator;
							if(match(um->chanmask,getName())) {
								if(um->hostmask[0] != 0 && um->modeflags & EModeFlags_BanExcempt)
									info->client->send_numeric(367,"%s *!*@%s %s@%s %d",getName(), um->hostmask, um->setbynick[0] == 0?"SERVER":um->setbynick,um->setbyhost[0] == 0||info->halfop==false?"*":um->setbyhost,um->setondate);
							}
						iterator++;
					}
					info->client->send_numeric(368,"%s :End of channel exempt list",getName());
					break;
				}
				find_nth(p,modep++,tbuff,sizeof(tbuff));
				if(strlen(tbuff)<1) return;
				char setstr[128];
				memset(&setstr,0, sizeof(setstr));
				tbuffptr = (char *)&tbuff;
				if(strchr(tbuffptr,'@') != NULL) tbuffptr = strchr(tbuffptr,'@')+1;
				sprintf(setstr,"\\hostmask\\%s\\modeflags\\E\\comment\\Exempt by MODE command (Persist)",tbuffptr);
				if(set) {
					addUserMode(info->client,this->getName(),setstr);
				} else {
					userMode *usermode = findRemoveableUsermode(getName(),tbuff,EModeFlags_BanExcempt,setter->getRights()&OPERPRIVS_GLOBALOWNER);
					if(usermode != NULL) {
						removeUsermode(usermode);
					}
				}
			} else {
				sendNotEnoughPrivs(ENotEnough_HALFOP,setter,"set mode");
			}
			break;
		default:
			setter->send_numeric(472,"%c :is unknown mode to me for %s",modestr[i],getName());
			break;
		}
	}
	return;
}
void Channel::sendTopic(Client *who) {
	if(this->topic[0] == 0) {
		who->send_numeric(331,"%s :No topic is set",getName());
	} else {
		who->send_numeric(332,"%s :%s",getName(),topic);
		who->send_numeric(333,"%s %s %i",getName(),topicsetter,topictime);
	}
	return;
}
void Channel::setTopic(Client *setter, char *str) {
	chanClient *info;
	chanClient tinfo;
	char *nick;
	bool set=true;
	memset(&tinfo,0,sizeof(tinfo));
	bool operoverride=false;
	if(setter->getRights()&OPERPRIVS_OPEROVERRIDE) operoverride=true;
	if(setter->getRights()&OPERPRIVS_GLOBALOWNER)  operoverride=true;
	if(!this->topic_protect) operoverride = true;
	info = getUserInfo(setter);
	if(info == NULL) info = &tinfo;
	if(!operoverride) {
		if(!info->halfop && !info->op && !info->owner) {
			if((this->onlyowner == 1 && !info->owner) || this->onlyowner == 0) {
				sendNotEnoughPrivs(ENotEnough_HALFOP,setter,"Set Topic");
				return;
			}
		}
	}
	memset(&this->topic,0,sizeof(this->topic));
	memset(&this->topicsetter,0,sizeof(this->topicsetter));
	strncpy(this->topic,str,strlen(str)%sizeof(this->topic));
	sendMessage(false,false,setter,"TOPIC %s :%s",getName(),this->topic);
	setter->getUserInfo(&nick,NULL,NULL,NULL);
	strcpy(this->topicsetter,nick);
	this->topictime = time(NULL);
}
void Channel::auditoriumUpdate(char oldMode) {
	std::list<chanClient *>::iterator it = user_list.begin();
	chanClient *info;
	while(it != user_list.end()) {
		info=*it;
		if(oldMode == 0) {
			if(this->auditorium == 1) { 
				if(info->halfop == false && info->op == false && info->owner == false && info->voice == false) {
					sendMessage(info->invisible,true,info->client,"PART %s :",getName());
				}
			} else if(this->auditorium == 2) { 
				sendMessage(info->invisible,true,info->client,"PART %s :",getName());
			} else { 
				sendMessage(info->invisible,true,info->client,"JOIN %s",getName());
				senduserModes(info);
			}
		} else if(oldMode == 1) { //was q
			if(this->auditorium == 0) { //reveal only non-ops
				if((info->halfop == false && info->op == false && info->owner == false && info->voice == false)) {
					sendMessage(info->invisible,true,info->client,"JOIN %s",getName());
					senduserModes(info);
				}
			} else { //hide only ops
				if(!(info->halfop == false && info->op == false && info->owner == false && info->voice == false)) {
					sendMessage(info->invisible,true,info->client,"PART %s :",getName());
				}
			}
		} else if(oldMode == 2) { //was u now q
			if(this->auditorium == 1) { //reveal only ops
				if(!(info->halfop == false && info->op == false && info->owner == false && info->voice == false)) {
					sendMessage(info->invisible,true,info->client,"JOIN %s",getName());
					senduserModes(info);
				}
			} else { //show all
					sendMessage(info->invisible,true,info->client,"JOIN %s",getName());
					senduserModes(info);
			}
		}
		it++;
	}
}
void Channel::senduserModes(chanClient *who) {
	void (Channel::*sendmethod)(char *fmt, ...);
	char *nick;
	who->client->getUserInfo(&nick,NULL,NULL,NULL);
	if(who->invisible) {
		sendmethod = &Channel::invisibleSend;
	} else {
		sendmethod = &Channel::sendToChan;
	}
	if(who->voice) {
		(this->*sendmethod)(":SERVER!SERVER@* MODE %s +v %s",getName(),nick);
	}
	if(who->halfop || who->op || who->owner) {
		(this->*sendmethod)(":SERVER!SERVER@* MODE %s +o %s",getName(),nick);
	}
}
void Channel::applyChanProps(chanProps *props,bool kickexisting) {
	if(kickexisting) {
		//delete the channel and have the modes be applied when someone joins next
		resetChannel(false,props==NULL?true:false);
		return;
	}
	if(props == NULL) {
		clearProps();
		return;
	}
	strcpy(entrymsg,props->entrymsg);
	if(strcmp(this->topic,props->topic) != 0) {
		strcpy(this->topic,props->topic);
		strcpy(this->topicsetter,"SERVER");
		this->topictime = props->setondate;
		this->createtime = props->setondate;
		sendToChan(":SERVER!SERVER@* TOPIC %s :%s",getName(),topic);
	}
	if(props->limit != 0) {
		if(props->limit != this->limit) {
			sendToChan(":SERVER!SERVER@* MODE %s +l %d",getName(),props->limit);
			this->limit = props->limit;
		}
	} else {
		if(this->limit != 0) {
			sendToChan(":SERVER!SERVER@* MODE %s -l",getName());
			this->limit = 0;
		}
	}
	if(props->chankey[0] != 0) {
		if(strcmp(props->chankey,key) != 0) {
			sendToChan(":SERVER!SERVER@* MODE %s +k %s",getName(),props->chankey);
			strcpy(key,props->chankey);
		}
	} else {
		if(this->key[0] != 0) {
			sendToChan(":SERVER!SERVER@* MODE %s -k",getName());
			key[0] = 0;
		}
	}
	#define setMode(modechar, modeval)  \
	if(strchr(props->modes,modechar) != NULL) { \
		if(modeval == 0) { \
			sendToChan(":SERVER!SERVER@* MODE %s +%c",getName(),modechar); \
			modeval = 1; \
		} \
		} else { \
		if(modeval == 1) { \
			sendToChan(":SERVER!SERVER@* MODE %s -%c",getName(),modechar); \
			modeval = 0; \
		} \
	}
	setMode('m',this->moderated)
	setMode('t',this->topic_protect)
	setMode('n',this->nooutsidemsgs)
	setMode('i',this->inviteonly)
	setMode('e',this->mode_ops_obey_channel_limit)
	#undef setMode
	if(strchr(props->modes,'u') != NULL) {
		char oldaudi = this->auditorium;
		if(oldaudi == 1) { //quiet
			sendToChan(":SERVER!SERVER@* MODE %s -q",getName());
		}
		sendToChan(":SERVER!SERVER@* MODE %s +u",getName());
		this->auditorium = 2;
		auditoriumUpdate(oldaudi);
	} else if(this->auditorium == 2) {
		sendToChan(":SERVER!SERVER@* MODE %s -u",getName());
		this->auditorium = 0;
		auditoriumUpdate(2);
	}
	if(strchr(props->modes,'q') != NULL) {
		char oldaudi = this->auditorium;
		if(oldaudi == 2) { //quiet
			sendToChan(":SERVER!SERVER@* MODE %s -u",getName());
		}
		sendToChan(":SERVER!SERVER@* MODE %s +q",getName());
		this->auditorium = 1;
		auditoriumUpdate(oldaudi);
	} else if(this->auditorium == 1) {
		sendToChan(":SERVER!SERVER@* MODE %s -q",getName());
		this->auditorium = 0;
		auditoriumUpdate(1);
	}
	if(strchr(props->modes,'s') != NULL) {
		if(this->priv != 0) {
			sendToChan(":SERVER!SERVER@* MODE %s -p",getName());
			this->priv = 0;
		}
		if(this->secret != 1) {
			sendToChan(":SERVER!SERVER@* MODE %s +s",getName());
			this->secret = 1;
		}
	}else if(this->secret == 1) {
		sendToChan(":SERVER!SERVER@* MODE %s -s",getName());
		this->secret = 0;
	}
	if(strchr(props->modes,'p') != NULL) {
		if(this->secret != 0) {
			sendToChan(":SERVER!SERVER@* MODE %s -s",getName());
			this->secret = 0;
		}
		if(this->priv != 1) {
			sendToChan(":SERVER!SERVER@* MODE %s +p",getName());
			this->priv = 1;
		}
	} else if(this->priv == 1) {
		sendToChan(":SERVER!SERVER@* MODE %s -p",getName());
		this->priv = 0;
	}
	this->onlyowner = props->onlyowner;
	this->stayopen = strchr(props->modes,'z') != NULL?true:false;
	this->registered = strchr(props->modes,'r') != NULL?true:false;
	this->allops = strchr(props->modes,'a') != NULL?true:false;
	if(props->groupname[0] != 0) {
		strcpy(this->groupname,props->groupname);
	}
	return;
}
void Channel::resetChannel(bool deleteatend, bool removed) {
	chanClient *user;
	char *nick;
	std::list<chanClient *>::iterator it = user_list.begin();
	while(it != user_list.end()) {
		user = *it;
		user->client->getUserInfo(&nick,NULL,NULL,NULL);
		if(user->invisible) {
			invisibleSend(":SERVER!SERVER@* KICK %s %s :Channel %s",getName(),nick,removed==true?"Removed":"Reset");
		} else {
			sendToChan(":SERVER!SERVER@* KICK %s %s :Channel %s",getName(),nick,removed==true?"Removed":"Reset");
		}
		this->removeUser(user->client,false,NULL,false);
		it = user_list.begin();
	}
	if(deleteatend) {
		deleteChannel(this);
	}
}
void Channel::clearProps() {
	this->entrymsg[0] = 0;
	this->groupname[0] = 0;
	if(this->topic[0] != 0) {
		sendToChan(":SERVER!SERVER@* TOPIC %s :",getName());
		this->topic[0] = 0;
	}
	#define unsetMode(modechar, modeval)  \
		if(modeval == 1) { \
			sendToChan(":SERVER!SERVER@* MODE %s -%c",getName(),modechar); \
			modeval = 0; \
		} 
	unsetMode('m',this->moderated)
	unsetMode('t',this->topic_protect)
	unsetMode('n',this->nooutsidemsgs)
	unsetMode('i',this->inviteonly)
	unsetMode('p',this->priv)
	unsetMode('s',this->secret)
	unsetMode('e',this->mode_ops_obey_channel_limit)
	#undef unsetMode
	if(this->auditorium != 0) {
		char oldaudi = this->auditorium;
		sendToChan(":SERVER!SERVER@* MODE %s -%c",getName(),oldaudi==1?'q':'u');
		this->auditorium = 0;
		auditoriumUpdate(oldaudi);
	}
	this->registered = false;
	this->onlyowner = false;
	this->allops = false;
	return;
}
 bool Channel::getChanKey(char *name, char *dst,int len, bool skip_userset) {
	 memset(dst,0,len);
	 if(stricmp(name,"topic") == 0) {
		 strncpy(dst,topic,strlen(topic)%len);
		 return true;
	 }
	 if(stricmp(name,"limit") == 0) {
		 snprintf(dst,len,"%d",limit);
		 return true;
	 }
	 if(stricmp(name,"key") == 0) {
		 snprintf(dst,len,"%d",this->key[0]!=0?1:0);
		 return true;
	 }
	if(stricmp(name,"groupname") == 0) {
		strncpy(dst,groupname,strlen(groupname)%len);
		return true;
	}
	 return skip_userset?NULL:getKey(chanKeys,name,dst,len);
 }
 char *Channel::getKeyBuff() {
	int alloclen = 1024;
	char *buff = (char *)malloc(alloclen);
	memset(buff,0,alloclen);
	if(topic[0] != 0) {
		strcat(buff,"\\topic\\");
		strcat(buff,topic);
	}
	if(strlen(buff) > (alloclen/2)) {
		alloclen += alloclen;
		buff = (char *)realloc(buff,alloclen);
	}
	if(limit != 0) {
		strcat(buff,"\\limit\\");
		char tbuff[32];
		sprintf(tbuff,"%d",limit);
		strcat(buff,tbuff);
	}
	if(key[0] != 0) {
		strcat(buff,"\\key\\1");
	}
	std::list<customKey *>::iterator iterator = chanKeys.begin();
	customKey *key;
	char tbuff[1024];
	while(iterator != chanKeys.end()) {
		key = *iterator;
		if(strlen(buff) > (alloclen/2)) {
			alloclen += alloclen;
			buff = (char *)realloc(buff,alloclen);
		}
		memset(&tbuff,0,sizeof(tbuff));
		snprintf(tbuff,sizeof(tbuff),"\\%s\\%s",key->name,key->value);
		strncat(buff,tbuff,strlen(tbuff)%alloclen);
		iterator++;
	}
	 return buff;
 }
 bool Channel::addParam(customKey* key) {
	customKey *curkey = getParamValue(key->name,NULL,0);
	if(key->name[0] == 'b' && key->name[1] == '_') { //broadcast flag
		this->sendToChan(":s 704 %s %s BCAST :\\%s\\%s",getName(),getName(),key->name,key->value);
	}
	if(curkey != NULL) {
		if(key->value[0] == 0) {
			chanKeys.remove(curkey);
			return false;
		}
		strcpy(curkey->value,key->value);
		return false;
	}
	customKey *newkey = (customKey *)malloc(sizeof(customKey));
	if(newkey != NULL) {
		memcpy(newkey,key,sizeof(customKey));
		chanKeys.push_back(newkey);
	}
	return true;
}
customKey *Channel::getParamValue(char *name, char *dst, int dstlen) {
	std::list<customKey *>::iterator iterator = chanKeys.begin();
	customKey *key;
	if(getChanKey(name, dst,dstlen,true)) {
		return NULL;
	}
	while(iterator != chanKeys.end()) {
		key = *iterator;
		if(stricmp(key->name,name) == 0) {
			if(dst != NULL && dstlen != 0)
				strncpy(key->value,dst,strlen(key->value)%dstlen);
			return key;
		}
		iterator++;
	}
	return NULL;
}
bool Channel::addUserParam(customKey* key, chanClient *user) {
	customKey *curkey = getParamValue(key->name,NULL,0);
	if(key->name[0] == 'b' && key->name[1] == '_') {
		void (Channel::*sendmethod)(char *fmt, ...);
		char *nick;
		user->client->getUserInfo(&nick,NULL,NULL,NULL);
		if(user->invisible) {
			sendmethod = &Channel::invisibleSend;
		} else {
			sendmethod = &Channel::sendToChan;
		}
		//:s 702 #gsp!!test #gsp!!test CHC BCAST :\b_lol
		char setval[256];
		memset(&setval,0,sizeof(setval));
		if(key->value[0] != 0) {
			snprintf(setval,sizeof(setval),"\\%s",key->value);
		} else {
			snprintf(setval,sizeof(setval),"\\");
		}
		(*this.*sendmethod)(":s 702 %s %s %s BCAST :\\%s%s",getName(),getName(),nick,key->name,setval);
	}
	if(curkey != NULL) {
		if(key->value[0] == 0) {
			user->userKeys->remove(curkey);
			return false;
		}
		strcpy(curkey->value,key->value);
		return false;
	}
	customKey *newkey = (customKey *)malloc(sizeof(customKey));
	if(newkey != NULL) {
		memcpy(newkey,key,sizeof(customKey));
		user->userKeys->push_back(newkey);
	} else return false;
	return true;
}
void Channel::clearnonGlobalModes() {
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *usermode;
	while(iterator != server.usermodes_list.end()) {
		usermode=*iterator;
		if(strcmp(usermode->chanmask,getName()) == 0) {
			if(usermode->isGlobal == 0) {
				server.usermodes_list.remove(usermode);
				iterator=server.usermodes_list.begin();
				continue;
			}
		}
		iterator++;
	}
}
