#include "client.h"
#include "channel.h"
#include "structs.h"
#include <common/helpers.h>
extern peerchatServer server;
bool Client::cmd_wallops(char *params) {
	char *p;
	if(params==NULL) {
		 goto notenoughparams;
	 }
	p = strchr(params,':');
	if(p==NULL) goto notenoughparams;
	sendWallops(this,p+1);
	return true;
	notenoughparams:
	 send_numeric(461,"WALLOPS :Not enough parameters");
	 return false;
}
bool Client::cmd_user(char *params) {
	 char *p;
	 if(params==NULL) {
		 goto notenoughparams;
	 }
	 p=params;
	 p = strchr(p,' ');
	 if(p==NULL) goto notenoughparams;
	 *p=0;
	 p = strchr(p+1,' ');
	 if(p==NULL) goto notenoughparams;
	 *p=0;
	 p = strchr(p+1,':');
	 if(p==NULL) goto notenoughparams;
	 p++;
	strncpy(this->user,params,strlen(params)%sizeof(user));
	strncpy(realname,p,strlen(p)%sizeof(realname));
	registered = nick[0] != 0;
	if(registered) {
		sendWelcome();
	}
	return true;
	notenoughparams:
	 send_numeric(461,"USER :Not enough parameters");
	 return false;
 }
 bool Client::cmd_nick(char *params) {
	 char *p;
	if(params == NULL) goto notenoughparams;
	 p=strchr(params,':');
	 if(p==NULL) {
		p = params;
	 } else
	 if(p != NULL) params = p+1;
	 strip(params,' ');
	 if(params[0] == '*' && params[1] == 0) { //set to uniquenick
		if(uniquenick[0] != 0) {
			sprintf(params,"%s-gs",uniquenick);
		} else {
			 send_numeric(432,"%s :Erroneous nickname",params);
			 return false;
		}
		if(nameInUse(params)) {
			Client *c = find_user(params);
			if(c != NULL) {//shouldn't be needed anyways
				c->quitUser("Duplicate Login"); //the actual server sends the person whos quitted a KILLED message saying it was them but it should be fine for other clients
			}
		}
	 } else {
		 if(!nameValid(params,true)) {
			 send_numeric(432,"%s :Erroneous nickname",params);
			 return false;
		 }
		 else if(nameInUse(params) || strcmp(params,"SERVER") == 0) {
			 send_numeric(433,"%s :Nickname is already in use",params);
			 return false;
		 }
	}
	 if(numChans(this) == 0 && registered) {
		 messageSend(this,"NICK :%s",params);
	 } else if(registered) {
		sendOnceAllChan(this,true,"NICK :%s",params); //must be sent before strcpy!
	 }
	 registered = user[0] != 0 && realname[0] != 0;
	 strncpy(nick,params,(strlen(params)+1)%sizeof(nick));
	 if(registered) {
		 sendWelcome();
	 }
	 return true;
	notenoughparams:
	 send_numeric(461,"NICK :Not enough parameters");
	 return false;
 }
 bool Client::cmd_setrights(char *params) {
	char *p,*target;
	Client *targetuser;
	if(params==NULL) {
		goto notenoughparams;
	}
	p = strchr(params, ' ');
        if(p == NULL) goto notenoughparams;	
	*p++=0;
	targetuser = find_user(params);
	if(targetuser == NULL) {
		send_numeric(401,"%s :No such nick/channel",params);
		return false;
	}
	targetuser->rightsmask=atoi(p);
	send_numeric(712,"Rightsmask set to %i (0x%08X)",targetuser->rightsmask,targetuser->rightsmask);
	return true;
	notenoughparams:
	 send_numeric(461,"SETRIGHTS :Not enough parameters");
	 return false;
 }
 bool Client::cmd_opermsg(char *params) {
	 char *p;
	 if(params==NULL) {
		 goto notenoughparams;
	 }
	 p = strchr(params,':');
	 if(p==NULL) {
		 p=params;
	 } else {
	 p++;
	 }
	 sendToAllOpers(":%s!%s@* NOTICE :OPERMSG: %s",nick,user,p);
	 return true;
	notenoughparams:
	 send_numeric(461,"OPERMSG :Not enough parameters");
	 return false;
 }
bool Client::cmd_listopers(char *params) {
	std::list<Client *>::iterator iterator;
	iterator=server.client_list.begin();
	Client *c;
	while(iterator != server.client_list.end()) {
		c=*iterator;
		if((c->getRights()  != 0)) {
			sendUserInfo(c,"LISTOPERS");
		}
		iterator++;
	}
	sendToClient(":SERVER!*@* PRIVMSG %s :LISTOPERS \\final\\1",nick);
	 return true;
 }
bool Client::cmd_listusers(char *params) {
	std::list<Client *>::iterator iterator;
	iterator=server.client_list.begin();
	Client *c;
	while(iterator != server.client_list.end()) {
		c=*iterator;
		sendUserInfo(c,"LISTUSERS");
		iterator++;
	}
	sendToClient(":SERVER!SERVER@* PRIVMSG %s :LISTUSERS \\final\\1",nick);
	 return true;
 }
bool Client::cmd_quit(char *params) {
	 char *p;
	 if(params==NULL) {
		 goto noreason;
	 }
	 p = strchr(params,':');
	 if(p==NULL) goto noreason;
	 if(strlen(p)<2) goto noreason;
	 quitUser(p);
	 return true;
noreason:
	 quitUser("Client Exited");
	return true;
}
bool Client::cmd_pingme(char *params) {
	if(params == NULL)
		sendToClient("PONG :s");
	else {
		if(params[0] == ':') {
			sendToClient(":s PONG s %s",params);
		} else {
			sendToClient(":s PONG s :%s",params);
		}
	}
	return true;
}

bool Client::cmd_crypt(char *params) {
//CRYPT des 1 thps6pc
	gameInfo *game;
	char servkey[17];
	char clikey[17];
	char *gamename;
	if(params == NULL) goto notenoughparams;
	gamename = strrchr(params, ' ');//assumes there isn't a space at the end of the gamename
	if(gamename == NULL) goto notenoughparams;
	gamename++;
	game = findGame(gamename);
	if(game == NULL) {
		quitUser("Invalid Game"); 
		return false;
	}
	if(game->servicesdisabled != 0) {
		char msg[128];
		switch(game->servicesdisabled) {
			case 1: {
				sprintf(msg, "Services Disabled: This game is disabled.");
				break;
			}
			default:
			case 2: {
				sprintf(msg, "Services Disabled: This game is temporarily disabled.");
				break;
			}
		}
		sendToClient("ERROR: Closing Link: s (%s)\r\n",msg);
		quitUser(msg);
	}
	gen_random(clikey,16);
	gen_random(servkey,16);
	send_numeric(705,"%s %s",clikey,servkey);
	gs_peerchat_init(&eServer, (unsigned char *)&servkey, (unsigned char *)game->secretkey);
	gs_peerchat_init(&eClient, (unsigned char *)&clikey, (unsigned char *)game->secretkey);
	encryted = true;
	gameid = game->id;
	 return true;
	notenoughparams:
	 send_numeric(461,"CRYPT :Not enough parameters");
	 return false;
}
bool Client::cmd_who(char *params) {
	char *p;
	Channel *targetchan;
	Client *targetuser;
	bool seeip;
	std::list<chanClient *> chanList;
	std::list<chanClient *>::iterator it;
	char statbuff[12];
	char *star = "*\0";
	chanClient *client;
	if(params == NULL) goto notenoughparams;
	p = strchr(params,' ');
	if(p != NULL) {
		*p = 0;
	}
	if(*params == ':') params++;
	if(params[0] == '#') { //channel
		targetchan = find_chan(params);
		if(targetchan == NULL) goto end;
		if(!targetchan->userOn(this) && ~getRights() & OPERPRIVS_OPEROVERRIDE && ((targetchan->secret == true || targetchan->priv == true))) {
			goto end;
		}
		chanList = targetchan->getList();
		it = chanList.begin();
		while(it != chanList.end()) {
			client = *it;
			if(client->invisible && ~getRights() & OPERPRIVS_INVISIBLE) {
				it++;
				continue;
			}
			if(targetchan->auditorium == 1 && !(client->halfop || client->op || client->owner || client->voice)) {
				it++;
				continue;
			}
			if(targetchan->auditorium == 2 && client->client != this) {
				it++;
				continue;
			}
			if(client->halfop || client->op || client->owner) {
				strcpy(statbuff,"H@");
			} else if(client->voice) {
				strcpy(statbuff,"H+");
			} else {
				strcpy(statbuff,"H");
			}
			char *hostname;
			hostname = client->client->host;
			if(!canSeeIp(this,client->client)) {
				hostname = star;
			}
			send_numeric(352,"%s %s %s s %s %s :0 %s",targetchan->getName(),client->client->user,hostname,client->client->nick,statbuff,client->client->realname);
			it++;
		}
	} else {
		targetuser = find_user(params);
		if(targetuser == NULL || (targetuser->getRights() & OPERPRIVS_HIDDEN && ~this->getRights() & OPERPRIVS_SEEHIDDEN)) goto end;
		char *hostname;
		hostname = targetuser->host;
		if(!canSeeIp(this,targetuser)) {
			hostname = star;
		}
		send_numeric(352,"* %s %s s %s H :0 %s",targetuser->user,hostname,targetuser->nick,targetuser->realname);
	}
	end:
	send_numeric(315,"%s :End of WHO list",params);
	 return true;
	notenoughparams:
	 send_numeric(461,"WHO :Not enough parameters");
	 return false;
}
bool Client::cmd_pong(char *params) {
	waiting_ping=false;
	return true;
}
bool Client::cmd_userhost(char *params) {
	Client *target;
	char *nick,*host,*user;
	char *star = "*\0";
	if(params == NULL) {
		target = this;
	} else {
		target = find_user(params);
		if(target == NULL || (target->getRights() & OPERPRIVS_HIDDEN && ~this->getRights() & OPERPRIVS_SEEHIDDEN)) {
			return false;
		}
	}
	target->getUserInfo(&nick,&user,&host,NULL);
	if(!canSeeIp(this,target) && target != this) {
		host = star;
	}
	sendToClient(":s 302 %s :%s=+%s@%s",this->nick,target->nick,target->user,host);
	return false;
}

bool Client::joinChan(char *name, chanClient *userInfo, char *key) { //specific channel(not with ,, etc)
	Channel *chan;
	if(name != NULL) {
		char *p = name;
		if(*p == ' ')
			while(*p++ == ' ');
		strip(p,' ');
		name = p;
		
	} else return false;
	if(!validChan(name)) {
		return false;
	}
	int modes = getUserChannelModes(this, name);
	if(modes & EModeFlags_Voice) userInfo->voice = true;
	if(modes & EModeFlags_HalfOp) userInfo->halfop = true;
	if(modes & EModeFlags_Op) userInfo->op = true;
	if(modes & EModeFlags_Owner) userInfo->owner = true;
	if(modes & EModeFlags_Gag) userInfo->gag = true;
	if(rightsmask&OPERPRIVS_GLOBALOWNER) userInfo->owner=true;
	if(rightsmask&OPERPRIVS_GETOPS) userInfo->op=true;
	if(rightsmask&OPERPRIVS_GETVOICE) userInfo->voice=true;
	if(modes & EModeFlags_Ban || !isClientAllowed(this,name)) {
		if(~rightsmask&OPERPRIVS_BANEXCEMPT) {
			if(!userInfo->op && !userInfo->owner && ~modes & EModeFlags_BanExcempt) {
				this->send_numeric(474,"%s :Cannot join channel (+b)",name);
				return false;
			}
		}
	}
	if(modes & EModeFlags_Invited) {
		userInfo->invited = true;
	}
	if(key != NULL) {
		if(tolower(key[0]=='x') && key[1] == ' ') { //extra, otherwise assumed channel key
			key=strchr(key,' ');
			if(key != NULL) {
				key++;
				for(int i=0;i<strlen(key);i++) {
					switch(key[i]) {
					case 'i':
						userInfo->invisible=true&&(rightsmask&OPERPRIVS_INVISIBLE);
						break;
					case 'q':
						userInfo->quiet=true;
						break;
					case 'g':
						userInfo->gag=true;
						break;
					case 'n':
						userInfo->voice=false;
						userInfo->halfop=false;
						userInfo->op=false;
						userInfo->owner=false;
						break;
					}
				}
			}
		}
	}
	chan=find_chan(name);
	bool createdChan = false;
	if(chan == NULL) { //must create the channel..
		chan=new Channel(name);
		createdChan = true;
		chanProps *props = getClosestChanProp(name);
		if(props != NULL) {
			chan->applyChanProps(props,false);
		}
		if(chan->registered == true) {
			userInfo->owner = true;
			userInfo->halfop = true;
			userInfo->op = true;
			char modestr[256];
			sprintf(modestr,"\\hostmask\\%s\\modeflags\\O\\isGlobal\\0\\comment\\Rogue Channel Creator",host);
			addUserMode(NULL,name,modestr,true);
		}

	}
	if(isUserInvited(this,chan)) {
		userInfo->invited = true;
	}
	if(chan->allops == true) {
			userInfo->owner = true;
			userInfo->halfop = true;
			userInfo->op = true;
			char modestr[256];
			sprintf(modestr,"\\hostmask\\%s\\modeflags\\o\\isGlobal\\0\\comment\\AllOps Channel",host);
			addUserMode(NULL,name,modestr,true);
	}
	if(chan == NULL) {
		send_numeric(473,"%s :Cannot join channel (Failed to allocate channel)",name);
		return false;
	}
	if(chan->inviteonly) {
		if(userInfo->invited != true && userInfo->op != true && userInfo->owner != true) {
			if(createdChan == true) {
				deleteChannel(chan);
			}
			send_numeric(473, "%s :Cannot join channel (+i)",name);
			return false;
		}
	}
	if(chan->limit != 0) {
		if(chan->getNumUsers() >= chan->limit) {
			if((userInfo->invited != true && userInfo->op != true && userInfo->owner != true) || chan->mode_ops_obey_channel_limit == true) {
				if(createdChan == true) {
					deleteChannel(chan);
				}
				send_numeric(471, "%s :Cannot join channel (+l)",name);
				return false;
			}
		}
	}
	if(chan->key[0] != 0) {
		if(key == NULL || strcmp(key,chan->key) != 0) {
			if(userInfo->op == false && userInfo->owner == false && userInfo->invited == false) {
				if(createdChan == true) {
					deleteChannel(chan);
				}
				send_numeric(475, "%s :Cannot join channel (+k)",name);
				return false;
			}
		}
	}
	if(numChans(this) > MAX_CHANS_PER_USER && ~getRights() & OPERPRIVS_OPEROVERRIDE) {
		if(createdChan == true) {
			deleteChannel(chan);
		}
		send_numeric(405,"%s :You have joined too many channels",name);
		return false;
	}
	if(chan->userOn(this)) {
		return false;
	}
	chan->addUser(userInfo);
	return true;
}
bool Client::cmd_join(char *params) {
	chanClient userInfo;
	char *name,*extra = NULL;
	int i;
	char *pch;
	if(params==NULL) {
		goto notenoughparams;
	}
	extra = strrchr(params, ',');
	if(extra != NULL) {
		extra++;
		while(*extra==' ') extra++;
		extra = strchr(extra, ' ');
		if(extra != NULL) extra++;
		else extra = NULL;
		if(extra != NULL)
			while(*extra==' ') extra++;
	} else {
		extra = strchr(params, ' ');
		if(extra != NULL) 
			while(*extra==' ') extra++;
	}
	if(extra != NULL) *(extra-1) = 0;
	name=params;
	memset(&userInfo,0,sizeof(chanClient));
	userInfo.client = this;
	pch = strtok(name,", ");
	i=0;
	while(pch != NULL) {
		joinChan(pch,&userInfo,extra);
		pch = strtok(NULL, ", ");
		i++;
	}
	if(i == 0) {
		joinChan(name,&userInfo,extra);
	}
	return true;
	notenoughparams:
	 send_numeric(461,"JOIN :Not enough parameters");
	 return false;
}

bool Client::cmd_fjoin(char *params) {
	char *name,*tchan;
	chanClient userInfo;
	Client *tuser;
	if(params==NULL) {
		goto notenoughparams;
	}
	tchan = strchr(params,' ');
	if(tchan != NULL) {
		*tchan = 0;
		tchan++;
	} else {
		goto notenoughparams;
	}
	name = params;
	tuser=find_user(name);
	if(tuser == NULL) {
		send_numeric(401,"%s :No such nick/channel",name);
		return false;
	}
	memset(&userInfo,0,sizeof(chanClient));
	userInfo.client = tuser;
	tuser->joinChan(tchan,&userInfo,NULL);
	return true;
	notenoughparams:
	 send_numeric(461,"FJOIN :Not enough parameters");
	 return false;
}
bool Client::partChan(char *name, char *reason) {
	Channel *chan;
	chan=find_chan(name);
	if(chan == NULL) {
		send_numeric(401,"%s :No such nick/channel",name);
		return false;
	}
	if(!chan->userOn(this)) {
		send_numeric(442,"%s :You're not on that channel",name);
		return false;
	}
	chan->removeUser(this,true,reason);
	return true;
}
bool Client::cmd_part(char *params) {
	char *reason;
	if(params == NULL) goto notenoughparams;
	reason = strchr(params,' ');
	if(reason != NULL) {
		*reason++ = 0;
		if(*reason == ':')
			reason++;
	}
	partChan(params,reason);
	return true;
	notenoughparams:
	 send_numeric(461,"PART :Not enough parameters");
	 return false;
}
void Client::sendNames(char *chan) {
	Channel *target;
	if(chan == NULL) return;
	target = find_chan(chan);
	if(target == NULL) return;
	if(target->userOn(this) || getRights()&OPERPRIVS_OPEROVERRIDE || (target->priv == false && target->secret == false) )
		target->sendNames(this);
	return;
}
bool Client::cmd_names(char *params) {
	if(params == NULL) goto notenoughparams;
	sendNames(params);
	return true;
	notenoughparams:
	 send_numeric(461,"NAMES :Not enough parameters");
	 return false;
}
void Client::sendModes(Client *who) {
	char sbuff[10];
	char *p;
	p=(char *)&sbuff;
	if(~getRights()&OPERPRIVS_MANIPULATE) {
		who = this;
	}
	memset(&sbuff,0,sizeof(sbuff));
	*p++='+';
	if(who->getModeFlags() & EUserMode_Quiet) {
		*p++='q';
	}
	if(who->getModeFlags() & EUserMode_ShowConns) {
		*p++ = 'c';
	}
	if(who->getModeFlags() & EUserMode_ShowJoins) {
		*p++ = 'j';
	}
	if(who->getModeFlags() & EUserMode_SpyMessages) {
		*p++ = 'p';
	}
	if(who->getModeFlags() & EUserMode_HideSpyMessages) { //hide pms sent to/from you
		*p++ = 's';
	}
	if(who->getModeFlags() & EUserMode_AllowInvisiblePrivmsg) {
		*p++ = 'n';
	}
	send_numeric(221,sbuff);
}
void Client::setModes(char *modestr) {
	bool set=true;
	for(int i=0;i<strlen(modestr);i++) {
		switch(modestr[i]) {
		case '-':
			set = false;
			break;
		case '+':
			set = true;
			break;
		case 'q':
			if(!set) {
				modeflags &= ~EUserMode_Quiet;
				sendAllChanNames(this);
			} else {
				modeflags |= EUserMode_Quiet;
			}
			break;
		case 'c':
			if(getRights()&OPERPRIVS_LISTOPERS) {
				if(set) {
					modeflags |= EUserMode_ShowConns;
				} else {
					modeflags &= ~EUserMode_ShowConns;
				}
			}
			break;
		case 'j':
			if(getRights()&OPERPRIVS_LISTOPERS) {
				if(set) {
					modeflags |= EUserMode_ShowJoins;
				} else {
					modeflags &= ~EUserMode_ShowJoins;
				}
			}
			break;
		case 'p':
			if(getRights()&OPERPRIVS_LISTOPERS) {
				if(set) {
					modeflags |= EUserMode_SpyMessages;
				} else {
					modeflags &= ~EUserMode_SpyMessages;
				}
			}
		case 's':
			if(getRights()&OPERPRIVS_MANIPULATE) {
				if(set) {
					modeflags |= EUserMode_HideSpyMessages;
				} else {
					modeflags &= ~EUserMode_HideSpyMessages;
				}
			}
			break;
		case 'n':
			if(getRights()&OPERPRIVS_INVISIBLE) {
				if(set) {
					modeflags |= EUserMode_AllowInvisiblePrivmsg;
				} else {
					modeflags &= ~EUserMode_AllowInvisiblePrivmsg;
				}
			}
		}
	}
}
bool Client::cmd_mode(char *params) {
	Channel *target;
	Client *usertarget;
	char *extra;
	if(params == NULL) goto notenoughparams;
	extra=strchr(params,' ');
	if(extra != NULL)
		*extra++=0;
	if(extra == NULL) { //send modes
		if(params[0]=='#') {
			target=find_chan(params);
			if(target == NULL) {
				//:s 403 Twix #test :No such channel
				send_numeric(403,"%s :No such channel",params);
				return false;
			}
			target->sendModes(this);
		} else {
			usertarget = find_user(params);
			if(usertarget == NULL) usertarget = this;
			sendModes(usertarget);
		}
	} else {
		if(params[0] == '#') {
			target=find_chan(params);
			if(target == NULL) {
				//:s 403 Twix #test :No such channel
				send_numeric(403,"%s :No such channel",params);
				return false;
			}
			target->setModes(this,extra);

		} else {
			usertarget = find_user(params);
			if(usertarget == NULL) usertarget = this;
			if(extra == NULL)
				sendModes(usertarget);
			else {
				setModes(extra);
			}
		}
	}
	return true;
	notenoughparams:
	 //send_numeric(461,"MODE :Not enough parameters");
	sendModes(this);
	 return false;
}
bool Client::cmd_cdkey(char *params) {
	send_numeric(706, "1 :Authenticated");
	return true;
}
bool Client::cmd_match(char *params) {
	char *extra;
	if(params == NULL) goto notenoughparams;
	extra=strchr(params,' ');
	if(extra != NULL)
		*extra++=0;
	send_numeric(666,"%s :%d %s %s",nick,match(params,extra),params,extra);
	return true;
notenoughparams:
	send_numeric(461,"MATCH :Not enough parameters");
	return false;
}
void Client::msgSend(char *name,bool no_ctcp, char *params) {
	Channel *tchan;
	Client *tuser;
	chanClient *info;
	char *target,*msg;
	target=strchr(params,' ');
	if(target == NULL)  return;
	*target = 0;
	msg = strchr(target+1,':');
	if(msg == NULL) return;
	msg++;
	target=params;
	if(no_ctcp && ~rightsmask & OPERPRIVS_CTCP) {
		if(msg[0]==0x01) { //possible ctcp
			if(strnicmp("ACTION",(msg+1),6) != 0 || *(msg+7) != ' ') return; //deny if isn't /me(its ctcp otherwise)
		}
	}
	if(gagged) return;
	if(target[0]=='#') {
		tchan = find_chan(target);
		if(tchan == NULL) {
			send_numeric(403,"%s :No such nick/channel",target);
			return;
		}
		info = tchan->getUserInfo(this);
		chanClient nullInfo;
		memset(&nullInfo,0,sizeof(chanClient));
		if(info == NULL) {
			info = &nullInfo;
		}
		bool invisible=(info->invisible && strcasecmp(name,"PRIVMSG") == 0 && (msg[0] != 0x01) || (msg[0] == 0x01 && strnicmp((msg+1),"ACTION",6) == 0 && *(msg+7) == ' '));
		if(modeflags & EUserMode_AllowInvisiblePrivmsg) {
			invisible = false;
		}
		if((tchan->moderated && (!info->voice && !info->halfop && !info->op && !info->owner) || info->gag) && (~rightsmask & OPERPRIVS_OPEROVERRIDE) || (tchan->nooutsidemsgs && (~rightsmask & OPERPRIVS_OPEROVERRIDE) && !tchan->userOn(this))) {
			send_numeric(404,"%s :Cannot send to channel",target);
			return;
		}
		tchan->sendMessage(invisible,true,this,"%s %s :%s",name,tchan->getName(),msg);
	} else {
		tuser = find_user(target);
		if(tuser == NULL) {
			send_numeric(403,"%s :No such nick/channel",target);
			return;
		}
		if(~tuser->modeflags & EUserMode_HideSpyMessages && ~modeflags & EUserMode_HideSpyMessages) {
		sendToAllWithMode((int)EUserMode_SpyMessages,":SERVER!SERVER@* NOTICE :%s sends a %s to %s: %s",nick,name,tuser->nick,msg);
		}
		if(strcasecmp(name,"PRIVMSG") == 0) {
			if(tuser->awaytext[0] != 0) {
				this->send_numeric(301,"%s :%s",tuser->nick,tuser->awaytext);
			}
		}
		char smsg[MAX_BUFF];
		sprintf_s(smsg,sizeof(smsg),"%s %s :%s",name,target,msg);
		tuser->messageSend(this,smsg);
	}
}
bool Client::cmd_privmsg(char *params) {
	if(params == NULL) goto notenoughparams;
	msgSend("PRIVMSG",true,params);
	return true;
	notenoughparams:
	 send_numeric(461,"PRIVMSG :Not enough parameters");
	 return false;
}
bool Client::cmd_notice(char *params) {
	if(params == NULL) goto notenoughparams;
	msgSend("NOTICE",false,params);
	return true;
	notenoughparams:
	 send_numeric(461,"NOTICE :Not enough parameters");
	 return false;
}
bool Client::cmd_atm(char *params) {
	if(params == NULL) goto notenoughparams;
	msgSend("ATM",false,params);
	return true;
	notenoughparams:
	 send_numeric(461,"ATM :Not enough parameters");
	 return false;
}
bool Client::cmd_utm(char *params) {
	if(params == NULL) goto notenoughparams;
	msgSend("UTM",false,params);
	return true;
	notenoughparams:
	 send_numeric(461,"UTM :Not enough parameters");
	 return false;
}
bool Client::cmd_topic(char *params) {
	Channel *chan;
	char *topic;
	if(params == NULL) goto notenoughparams;
	topic = strchr(params,' ');
	if(topic != NULL) {
	*topic = 0;
	topic++;
	if(*topic == ':') topic++;
	}
	chan = find_chan(params);
	if(topic == NULL) {
		chan->sendTopic(this);
		return true;
	}
	chan->setTopic(this,topic);
	return true;
	notenoughparams:
	 send_numeric(461,"TOPIC :Not enough parameters");
	 return false;
}
void Client::sendWhois(Client *target) {
	bool seeip=canSeeIp(this,target);
	int numchans=numChans(target);
	char star[]= {'*','\0'};
	char *nick,*user,*host,*realname;
	target->getUserInfo(&nick,&user,&host,&realname);
	if(!seeip) host = (char *)&star;
	send_numeric(311,"%s %s %s * :%s",nick,user,host,realname);
	if(numchans > 0) {
		std::list<Channel *>::iterator iterator = server.chan_list.begin();
		Channel *chan;
		char *sendbuff;
		int len = 1024;
		int i=0,numvisible = 0;
		// :s 319 CHC CHC :#1 #a #t #b #u #e #q #aya #asasdat #asdadatrastas #lololol #testestsetset #lololaaa #rawrrrrrrrr #loatase #cookies
		sendbuff = (char *)malloc(len);
		sprintf(sendbuff,"%s :",nick);
		chanClient *user;
		bool opoverride = getRights()&OPERPRIVS_OPEROVERRIDE;
		while(iterator != server.chan_list.end()) {
			chan=*iterator;
			if(!opoverride && ((chan->priv == 1 || chan->secret == 1) && !chan->userOn(this))) {
				iterator++;
				continue;
			}
			if(chan->userOn(target)) {
				user = chan->getUserInfo(target);
				if(user->invisible && ~getRights() & OPERPRIVS_INVISIBLE) {
					iterator++;
					continue;
				} else if(user->halfop || user->op || user->owner) {
					strcat((char *)sendbuff, "@");
				} else if(user->voice) {
					strcat((char *)sendbuff, "+");
				}
				numvisible++;
				strcat((char *)sendbuff, chan->getName());
				if(numchans != i+1)
					strcat((char *)sendbuff, " ");
			}
			if(strlen(sendbuff) > (len/2)) {
				len *=2;
				sendbuff = (char *)realloc(sendbuff,len);
			}
			i++;
			iterator++;
		}
		if(numvisible > 0) {
			send_numeric(319,"%s",sendbuff);
		}
		free((void *)sendbuff);
	}
}
bool Client::cmd_whois(char *params) {
	char *name;
	Client *target;
	if(params == NULL) goto notenoughparams;
	name = strchr(params,' ');
	if(name != NULL)
		*name = 0;
	name = params;
	target=find_user(name);
	if(target == NULL || (target->getRights() & OPERPRIVS_HIDDEN && ~this->getRights() & OPERPRIVS_SEEHIDDEN)) {
		send_numeric(401,"%s :No such nick/channel",name);
		goto end;
	}
	sendWhois(target);
end:
	send_numeric(318,"%s :End of WHOIS list",name);
	return true;
	notenoughparams:
	 send_numeric(431,"No nickname given");
	 return false;
}
bool Client::cmd_setusermode(char *params) {
	char data[128];
	Channel *chan;
	char *setstr;
	if(params == NULL) goto notenoughparams;
	setstr = strchr(params,' ');
	if(setstr == NULL) goto notenoughparams;
	*setstr++ = 0;
	if(strchr(params,'*') != NULL) {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			send_numeric(482,"%s :You're not channel operator (Wildcard set - OP_GETS_OPS)",params);
			return false;
		}
	}
	if(tolower(params[0]) == 'x' && params[1] == 0) {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			return false;
		}
	}
	addUserMode(this, params, setstr,true);
	return true;
	notenoughparams:
	 send_numeric(461,"SETUSERMODE :Not enough parameters");
	 return false;
}
bool Client::cmd_listusermodes(char *params) {
	std::list<userMode *>::iterator iterator=server.usermodes_list.begin();
	userMode *um;
	if(params == NULL) goto notenoughparams;
	if(tolower(params[0]) == 'x' && params[1] == 0) {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			return false;
		}
	}
	if(strchr(params,'*') != NULL) {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			send_numeric(482,"%s :You're not channel operator (Wildcard list - OP_GETS_OPS)",params);
			return false;
		}
	}
	if(~getRights() & OPERPRIVS_GLOBALOWNER) {
		Channel *chan;
		chan = find_chan(params);
		if(chan == NULL) {
			send_numeric(403,"%s :No such channel",params);
			return false;
		}
		chanClient *info;
		info = chan->getUserInfo(this);
		if(info == NULL || (!info->halfop && !info->op && !info->owner)) {
			send_numeric(482,"%s :You're not channel operator (list modes - HALFOPS)",params);
			return false;
		}
	}
	while(iterator != server.usermodes_list.end()) {
		um=*iterator;
		if(match(params,um->chanmask) == 0)
			sendUserMode(this,um);
		iterator++;
	}
	sendToClient(":SERVER!SERVER@* PRIVMSG %s :LISTUSERMODE \\final\\1",nick);
	return true;
	notenoughparams:
	 send_numeric(461,"LISTUSERMODES :Not enough parameters");
	return false;
}
bool Client::cmd_delusermode(char *params) {
	//the 1st param is ignored for some stupid reason on peerchat so skip it and search by usermodeid
	chanClient *client;
	Channel *chan;
	char *setstr;
	int usermodeid;
	userMode *usermode;
	if(params == NULL) goto notenoughparams;
	setstr = strchr(params,' ');
	if(setstr == NULL) goto notenoughparams;
	*setstr++ = 0;
	usermodeid = atoi(setstr);
	usermode = getUserMode(usermodeid);
	if(usermode == NULL) {
		this->sendToClient(":SERVER!SERVER@* PRIVMSG %s :DELUSERMODE usermodeid not found",nick);
		return false;
	}
	if(strchr(usermode->chanmask,'*') != NULL) {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			sendToClient(":SERVER!SERVER@* PRIVMSG %s :%s is a wildcard mask (must be cleared by OP_GET_OPS)",nick,usermode->chanmask);
			return false;
		}
	} else if(tolower(usermode->hostmask[0]) == 'x' && usermode->hostmask[1] == 0) {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			return false;
		}
	} else {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			chan = find_chan(usermode->chanmask);
			if(chan == NULL) {
				send_numeric(403,"%s :No such channel",params);
				return false;
			}
			client = chan->getUserInfo(this);
			if(client == NULL) {
				send_numeric(482,"%s :You're not channel operator (Del Mode - HALFOPS)",chan->getName());
				return false;
			}
			if(usermode->modeflags & EModeFlags_Op && client->op == false) {
				send_numeric(482,"%s :You're not channel operator (Del Op - Op)",chan->getName());
				return false;
			}
			else if(usermode->modeflags & EModeFlags_HalfOp && client->op == false) {
				send_numeric(482,"%s :You're not channel operator (Del HalfOp - Op)",chan->getName());
				return false;
			}
			else if(usermode->modeflags & EModeFlags_Owner && client->owner == false) {
				send_numeric(482,"%s :You're not channel operator (Del Owner - Owner)",chan->getName());
				return false;
			}
			else if(client->halfop == false) {
				send_numeric(482,"%s :You're not channel operator (Del Mode - HALFOPS)",chan->getName());
				return false;
			}
		}
	}
	
	removeUsermode(usermode);
	return true;
	notenoughparams:
	 send_numeric(461,"DELUSERMODE :Not enough parameters");
	 return false;
}
/*<SERVER> LISTCHANPROPS \chanmask\#gsp!cc_84589986_a63ad0\onlyowner\1\expires\None\setondate\07/13/2008 16:56\setbypid\84589986\setbynick\nick\comment\Chat Club\mode\z\topic\[Guard]Falcon's nest||GEN|0
 <SERVER> LISTCHANPROPS \final\1*/
bool Client::cmd_listchanprops(char *params) {
	chanProps *um;
	if(params == NULL) goto notenoughparams;
	um = getClosestChanProp(params);
	if(um != NULL) {
		sendChanProps(this, um);
	}
	/*
	while(iterator != server.chanprops_list.end()) {
		um=*iterator;
		if(match(params,um->chanmask) == 0) {
			sendChanProps(this,um);
		}
		iterator++;
	}
	*/
	sendToClient(":SERVER!SERVER@* PRIVMSG %s :LISTCHANPROPS \\final\\1",nick);
	return true;
	notenoughparams:
	 send_numeric(461,"LISTCHANPROPS :Not enough parameters");
	 return false;
}
bool Client::cmd_delchanprops(char *params) {
	Channel *chan;
	bool resetchan = false;
	char *setstr;
	chanProps *props;
	chanClient *user;
	if(params == NULL) goto notenoughparams;
	setstr = strchr(params,' ');
	if(setstr != NULL) {
		*setstr++ = 0;
		resetchan = atoi(setstr)==1?true:false;
	}
	props = getClosestChanProp(params);
	// <SERVER> #gsp!*!*  is a wildcard mask (must be cleared by wildcard name)
	if(props == NULL) {
		sendToClient(":SERVER!SERVER@* PRIVMSG %s :No matching chanprops found",nick);
		return false;
	} else if(stricmp(props->chanmask,params) != 0) {
		if(strchr(props->chanmask,'*') != NULL) {
			sendToClient(":SERVER!SERVER@* PRIVMSG %s :%s is a wildcard mask (must be cleared by wildcard name)",nick,props->chanmask);
			return false;
		} else {
			sendToClient(":SERVER!SERVER@* PRIVMSG %s :%s doesn't match %s",nick,props->chanmask,params);
			return false;
		}
	}
	if(~getRights() & OPERPRIVS_GLOBALOWNER) {
		chan = find_chan(params);
		if(chan == NULL) {
			//:s 482 CHC #gsp!mkvsdcps3_m1a :You're not channel operator (Del Props - OWNER)
			send_numeric(403,"%s :No such channel",params);
			return false;
		}
		user = chan->getUserInfo(this);
		if(user == NULL || user->owner == false) {
			send_numeric(482,"%s :You're not channel operator (Del Props - OWNER)",params);
			return false;
		}
	}
	deleteChanProp(params, resetchan);
	return true;
	notenoughparams:
	 send_numeric(461,"DELCHANPROPS :Not enough parameters");
	 return false;
}
bool Client::cmd_kill(char *params) {
	char setstr[128];
	char *nullchar = "\0";
	char *reason;
	char *host;
	char killmsg[256];
	Client *target;
	if(params == NULL) goto notenoughparams;
	reason = strchr(params,' ');
	if(reason != NULL) {
		*reason++ = 0;
		if(*reason == ':') {
			reason++;
		}
	}
	target = find_user(params);
	if(target == NULL) {
		send_numeric(403,"%s :No such nick/channel",params);
		return false;
	}
	if(reason == NULL) {
		reason = nullchar;
	}
	target->getUserInfo(NULL,NULL,&host,NULL);
	sprintf(killmsg,"KILLED by %s: %s",nick,reason);
	target->quitUser(killmsg);
	sprintf(setstr,"\\hostmask\\%s\\modeflags\\b\\comment\\Killed\\expiressec\\600",host);
	addUserMode(this,"X",setstr,true);
	return true;
	notenoughparams:
	 send_numeric(461,"KILL :Not enough parameters");
	return false;
}
bool Client::cmd_kline(char *params) {
	char *host;
	if(params == NULL) goto notenoughparams;
	host = strchr(params,' ');
	if(host != NULL) {
		*host++ = 0;
		if(*host == ':') {
			host++;
		}
	}
	char setstr[128];
	sprintf(setstr,"\\hostmask\\%s\\modeflags\\b\\comment\\Killed\\expiressec\\600",host);
	addUserMode(this,"X",setstr,true);
	return true;
	notenoughparams:
	 send_numeric(461,"KLINE :Not enough parameters");
	return false;
}
bool Client::cmd_setchanprops(char *params) {
	char data[128];
	Channel *chan;
	char *setstr;
	if(params == NULL) goto notenoughparams;
	setstr = strchr(params,' ');
	if(setstr == NULL) goto notenoughparams;
	*setstr++ = 0;
	if(strchr(params,'*') != NULL) {
		if(~getRights() & OPERPRIVS_GLOBALOWNER) {
			send_numeric(482,"%s :You're not channel operator (Wildcard set - OP_GLOBAL_OWNER)",params);
			return false;
		}
	}
	addChanProp(this,params,setstr);
	return true;
	notenoughparams:
	send_numeric(461,"SETCHANPROPS :Not enough parameters");
	return false;
}
bool Client::cmd_setgroup(char *params) {
	char *nullchar = "\0";
	char *reason;
	Channel *target;
	chanClient *info;
	if(params == NULL) goto notenoughparams;
	reason = strchr(params,' ');
	if(reason != NULL) {
		*reason++ = 0;
		if(*reason == ':') {
			reason++;
		}
	} else {
		reason = nullchar;
	}
	target = find_chan(params);
	if(target == NULL) {
		send_numeric(401, "%s :No such nick/channel",params);
		return false;
	}
	info = target->getUserInfo(this);
	if(info == NULL) {
		send_numeric(442, "%s :You're not on that channel",target->getName());
		return false;
	}
	if(~getRights() & OPERPRIVS_OPEROVERRIDE) {
		if(info->halfop == false || info->op == false || info->owner == false) {
			send_numeric(482, "%s :You're not channel operator (Set Group - HALFOPS)",target->getName());
			return false;
		}
	}
	strcpy(target->groupname,reason);
	return true;

	notenoughparams:
	send_numeric(461,"SETGROUP :Not enough parameters");
	return false;
}
bool Client::cmd_listchans(char *params) {
	char *chanpattern = params;
	bool specialinfo = false;
	bool patterned = true;
	char *extra = NULL;
	bool showall;
	int numusers;
	Channel *chan;
	std::list<Channel *>::iterator iterator;
	if(params != NULL) {
		extra = strchr(params,' ');
	}
	if(extra != NULL) {
		*extra++ = 0;
	}
	if(extra == NULL) {
		if(params != NULL) {
			if(tolower(params[0]) == 'k' && params[1] == 0) {
				specialinfo = true;
			}
		}
	} else {
		if(tolower(extra[0]) == 'k' && extra[1] == 0) {
			specialinfo = true;
		}
	}
	send_numeric(321, "Channel :Users Name");
	iterator=server.chan_list.begin();
	showall = false;
	if(getRights() & OPERPRIVS_OPEROVERRIDE) {
		showall = true;
	}
	while(iterator != server.chan_list.end()) {
		chan = *iterator;
		numusers = chan->getNumUsers(showall);
		if(showall || (!chan->priv && !chan->secret) || chan->userOn(this)) {
			//:s 322 CHC #GSP!civ4bts!M3313aJDDM 1 :\topic\Garen\limit\32
			if(numusers == 0 && !chan->stayopen)  {
				iterator++;
				continue;
			}
			if(params != NULL) {
				if(match(params, chan->getName()) != 0) {
					if(strcmp(params,chan->groupname) != 0) {
						iterator++;
						continue;
					}
				}
			}
			char *topic = (char *)&chan->topic;
			if(specialinfo)
				topic = chan->getKeyBuff();
			send_numeric(322, "%s %d :%s",chan->getName(),numusers,topic);
			if(specialinfo)
				free((void *)topic);
		}
		iterator++;
	}
	send_numeric(323, ":End of LIST");
	return true;
}
bool Client::cmd_whowas(char *params) {
	int i=0;
	std::list<whowasInfo *>::iterator iterator = server.whowas_list.begin();
	whowasInfo *info;
	char timestr[64];
	while(iterator != server.whowas_list.end()) {
		info = *iterator;
		if(match(params,info->name) == 0) {
			struct tm * timeinfo;
			timeinfo = localtime ( &info->quittime );
			strftime(timestr,sizeof(timestr),"%m/%d/%Y %H:%M",timeinfo);
			send_numeric(314,"%s %s %s * :%s",info->name,info->username,info->host,info->realname);
			send_numeric(312,"%s s :%s",info->name,timestr);
			i++;
		}
		iterator++;
	}
	if(i == 0) {
		send_numeric(406,"%s :There was no such nickname",params);
	}
	send_numeric(369,"%s :End of WHOWAS",params);
	return true;
	notenoughparams:
	send_numeric(461,"WHOWAS :Not enough parameters");
	return false;
}
bool Client::cmd_setkey(char *params) {
	char *setstr;
	if(params == NULL) goto notenoughparams;
	setstr = strchr(params,':');
	if(setstr == NULL) goto notenoughparams;
	*setstr++=0;
	char param[MAX_COMMENT];
	customKey key;
	memset(&key,0,sizeof(customKey));
	int i;
	for(i=0;find_param(i,setstr,(char *)&param,sizeof(param));i++) {
		if(i%2!= 1) { //option
			strcpy(key.name,param);
		} else {
			strcpy(key.value,param);
			addUserParam(&key);
		}
	}
	if(i%2 == 1) { //last param was to be removed
		addUserParam(&key);
	}
	return true;
	notenoughparams:
	send_numeric(461,"SETKEY :Not enough parameters");
	return false;
}
bool Client::cmd_getkey(char *params) {
	//:s 700 CHC CHC 0 :\1852\0
	//GETKEY CHC 0 000 :\gameid\profileid
	Client *target;
	char *identnum,*ident2,*query,*response,*pch;
	int curlen;
	char value[256];
	if(params == NULL) goto notenoughparams;
	identnum = strchr(params,' '); //ident number(used in response only) can be text too though
	if(identnum == NULL) goto notenoughparams;
	*identnum++ = 0;
	ident2 = strchr(identnum, ' ');
	if(ident2 == NULL) goto notenoughparams;
	*ident2++ = 0;
	query = strchr(ident2, ':');
	if(query == NULL) goto notenoughparams;
	*query++=0;
	target = find_user(params);
	if(target == NULL || (target->getRights() & OPERPRIVS_HIDDEN && ~this->getRights() & OPERPRIVS_SEEHIDDEN)) {
		send_numeric(401,"%s :No such nick/channel",params);
		return false;
	}
	response = (char *)malloc(1024);
	curlen = snprintf(response,1024,":s 700 %s %s %s :",nick,target->nick,identnum);
	pch = strtok (query,"\\");
	while(pch != NULL) {
		if(!target->getUserKey(pch, (char *)value, sizeof(value))) {
			getKey(target->userKeys,pch,(char *)&value,sizeof(value));
		}
		if(strlen(value) + sizeof(value) + curlen > strlen(response)) response = (char *)realloc(response, strlen(response)+strlen(value) + curlen + sizeof(value) + 1024);
		strcat(response,"\\");
		strcat(response,value);
		pch = strtok (NULL,"\\");
	}
	sendToClient("%s",response);
	free((void *)response);
	return true;
	notenoughparams:
	send_numeric(461,"GETKEY :Not enough parameters");
	return false;
}
bool Client::cmd_setchankey(char *params) {
	////setchankey $chan :\b_test\lol
	Channel *target;
	char *setstr;
	char param[MAX_COMMENT];
	customKey key;
	if(params == NULL) goto notenoughparams;
	setstr = strchr(params,':'); //ident number(used in response only) can be text too though
	if(setstr == NULL) goto notenoughparams;
	*setstr++=0;
	strip(params,' '); //kill the spaces!
	target = find_chan(params);
	if(target == NULL) {
		return false;
	}
	if(~getRights()&OPERPRIVS_OPEROVERRIDE) {
		chanClient *info = target->getUserInfo(this);
		if(info == NULL || (info->halfop == false && info->op == false && info->owner == false)) {
			return false;
		}
	}
	memset(&key,0,sizeof(customKey));
	int i;
	for(i=0;find_param(i,setstr,(char *)&param,sizeof(param));i++) {
		if(i%2!= 1) { //option
			strcpy(key.name,param);
		} else {
			strcpy(key.value,param);
			target->addParam(&key);
		}
	}
	if(i%2 == 1) { //last param was to be removed
		target->addParam(&key);
	}
	return true;
	notenoughparams:
	send_numeric(461,"SETCHANKEY :Not enough parameters");
	return false;
}
// :s 704 CHC #gsp!!test 0 :\cookies
bool Client::cmd_getchankey(char *params) {
	char *identnum,*identnum2,*getstr;
	char *response,*pch;
	int curlen;
	Channel *chan;
	////getchankey $chan 0 000 :\test
	// :s 704 #gsp!!test #gsp!!test BCAST :\b_owned\1
	if(params == NULL) goto notenoughparams;
	identnum = strchr(params,' '); //ident number(used in response only) can be text too though
	if(identnum == NULL) goto notenoughparams;
	*identnum++ = 0;
	identnum2 = strchr(identnum,' '); //ident number(used in response only) can be text too though
	if(identnum2 == NULL) goto notenoughparams;
	*identnum2++ = 0;
	getstr = strchr(identnum2,':'); //ident number(used in response only) can be text too though
	if(getstr != NULL) {
		*getstr++ = 0;
	}
	chan = find_chan(params);
	if(chan == NULL) {
		send_numeric(401,"%s :No such nick/channel",params);
		return false;
	}
	if(~getRights() & OPERPRIVS_OPEROVERRIDE) {
		if(!chan->userOn(this)) {
			if(chan->priv || chan->secret) {
				return false;
			}
		}
	}
	
	response = (char *)malloc(1024);
	//s 704 Shinto #gsp!!test 0 :\lol
	curlen = snprintf(response,1024,":s 704 %s %s %s :",nick,chan->getName(),identnum);
	if(getstr == NULL) { //list out all chankey stuff
		char *keybuff;
		keybuff = chan->getKeyBuff();
		strcat(response,keybuff);
		sendToClient("%s",response);
		free((void *)keybuff);
		return true;
	}
	pch = strtok (getstr,"\\");
	char value[256];
	while(pch != NULL) {
		chan->getChanKey(pch,(char *)value, sizeof(value));
		if(strlen(value) + sizeof(value) + curlen > strlen(response)) response = (char *)realloc(response, strlen(response)+strlen(value) + curlen + sizeof(value) + 1024);
		strcat(response,"\\");
		strcat(response,value);
		pch = strtok (NULL,"\\");
	}
	sendToClient("%s",response);
	free((void *)response);
	//todo: check if atleast halfop
	//sprintf(buffer, "GETCHANKEY %s %s 0 :", channel, cookie);
	return true;
	notenoughparams:
	send_numeric(461,"GETCHANKEY :Not enough parameters");
	return false;
}
bool Client::cmd_getckey(char *params) {
	//GETCKEY %s %s %s 0 :", channel, nick, cookie);
	char *target,*cookie,*cookie2,*getstr;
	Client *targetuser;
	Channel *chan;
	chanClient *client;
	int querycplen;
	char *querycp;
	int numsent = 0;
	if(params == NULL) goto notenoughparams;
	target = strchr(params, ' ');
	if(target == NULL) goto notenoughparams;
	*target++ = 0;
	cookie = strchr(target, ' ');
	if(cookie == NULL) goto notenoughparams;
	*cookie++ = 0;
	cookie2 = strchr(cookie, ' ');
	if(cookie2 == NULL) goto notenoughparams;
	*cookie2++ = 0;
	getstr = strchr(cookie2, ' ');
	if(getstr == NULL) goto notenoughparams;

	*getstr++ = 0;
	if(*getstr==':') getstr++;
	querycplen = strlen(getstr)+1;
	querycp = (char *)malloc(querycplen);
	memset(querycp,0,querycplen);
	strcpy(querycp,getstr);
	
	chan = find_chan(params);
	if(chan == NULL) {
		//peerchat doesn't give an error for this so we don't either
		return false;
	}
	client = NULL;
	if(strcmp(target,"*") != 0) {
		targetuser = find_user(target);
		//:s 702 Shinto #gsp!jbnightfire Falcon 0 :\falcon
		if(targetuser == NULL) {
			//s 401 Shinto Shinto :No such nick/channel
			send_numeric(401,"%s :No such nick/channel",target);
			return false;
		}
		client = chan->getUserInfo(targetuser);
		if(client == NULL || (client->invisible && client->client->getRights() & OPERPRIVS_INVISIBLE && ~getRights() & OPERPRIVS_INVISIBLE)) {
			send_numeric(401,"%s :No such nick/channel",target);
			return false;
		}
	}
	if(client != NULL) { //specific client lookup
		numsent++;
		char *response = (char *)malloc(1024);
		//:s 702 Shinto #gsp!!test CHC 0 :\XFOWvpDvpX|0\\7557fac37091291343e9b28c0eb8e59f
		int curlen = snprintf(response,1024,":s 702 %s %s %s %s :",nick,params,client->client->nick,cookie);
		char * pch;
		pch = strtok (getstr,"\\");
		char value[256];
		while(pch != NULL) {
			if(!targetuser->getUserKey(pch, (char *)value, sizeof(value))) {
				getKey(client->userKeys,pch,(char *)&value,sizeof(value));
			}
			if(strlen(value) + sizeof(value) + curlen > strlen(response)) response = (char *)realloc(response, strlen(response)+strlen(value) + curlen + sizeof(value) + 1024);
			strcat(response,"\\");
			strcat(response,value);
			pch = strtok (NULL,"\\");
	}
	sendToClient("%s",response);
	free((void *)response);
	} else { //print all clients
		std::list<chanClient *> clientList = chan->getList();
		std::list<chanClient *>::iterator it = clientList.begin();
		while(it != clientList.end()) {
			client = *it;
			if(client->invisible && ~getRights()&OPERPRIVS_INVISIBLE) {
				it++;
				continue;
			}
			char *response = (char *)malloc(1024);
			//:s 702 Shinto #gsp!!test CHC 0 :\XFOWvpDvpX|0\\7557fac37091291343e9b28c0eb8e59f
			int curlen = snprintf(response,1024,":s 702 %s %s %s %s :",nick,params,client->client->nick,cookie);
			char * pch;
			strcpy(getstr,querycp);
			pch = strtok (getstr,"\\");
			char value[256];
			while(pch != NULL) {
				if(!client->client->getUserKey(pch, (char *)value, sizeof(value))) {
					getKey(client->userKeys,pch,(char *)&value,sizeof(value));
				}
				if(strlen(value) + sizeof(value) + curlen > strlen(response)) response = (char *)realloc(response, strlen(response)+strlen(value) + curlen + sizeof(value) + 1024);
				strcat(response,"\\");
				strcat(response,value);
				pch = strtok (NULL,"\\");
			}
		numsent++;
		sendToClient("%s",response);
		free((void *)response);
		it++;
		}
	}
	if(numsent > 0)
		send_numeric(703,"%s %s :End of GETCKEY",chan->getName(),cookie);
	return true;
	notenoughparams:
	send_numeric(461,"GETCKEY :Not enough parameters");
	return false;
}
bool Client::cmd_setckey(char *params) {
	////setckey $chan $me :\b_test\1
	char *setstr;
	char *target;
	Channel *chan;
	Client *user;
	chanClient *client;
	char param[MAX_COMMENT];
	customKey key;
	int i;
	if(params == NULL) goto notenoughparams;
	target = strchr(params, ' ');
	if(target == NULL) goto notenoughparams;
	*target++=0;
	setstr = strchr(target, ':');
	if(setstr == NULL) goto notenoughparams;
	*setstr++=0;
	strip(target,' ');
	chan = find_chan(params);
	if(chan == NULL) {
		return false;
	}
	user = find_user(target);
	if(user == NULL || (user != this && ~getRights()&OPERPRIVS_MANIPULATE)) {
		return false;
	}
	client = chan->getUserInfo(user);
	if(client == NULL) {
		return false;
	}
	memset(&key,0,sizeof(customKey));
	for(i=0;find_param(i,setstr,(char *)&param,sizeof(param));i++) {
		if(i%2!= 1) { //option
			strcpy(key.name,param);
		} else {
			strcpy(key.value,param);
			chan->addUserParam(&key,client);
		}
	}
	if(i%2 == 1) { //last param was to be removed
		chan->addUserParam(&key,client);
	}
	return true;
	notenoughparams:
	send_numeric(461,"SETCKEY :Not enough parameters");
	return false;
}
bool Client::cmd_kick(char *params) {
	//KICK #gsp!!test CHC :test
	Channel *chan;
	Client *target;
	char *targetnick = strchr(params,' ');
	bool opoverride;
	char *reason;
	if(targetnick == NULL) goto notenoughparams;
	*targetnick++=0;
	chan = find_chan(params);
	if(chan == NULL) {
		//:s 403 CHC #gsp!aaaaaaaaaaa :No such channel
		send_numeric(403,"%s :No such channel",params);
		return false;
	}
	reason = strchr(targetnick, ' ');
	if(reason != NULL) {
		*reason++=0;
		if(*reason == ':') {
			reason++;
		}
	}
	target = find_user(targetnick);
	if(target == NULL) {
		send_numeric(441,"%s :They aren't on that channel",targetnick);
		return false;
	}
	chanClient *selfprivs;
	chanClient *userprivs;
	selfprivs = chan->getUserInfo(this);
	userprivs = chan->getUserInfo(target);
	if(userprivs == NULL || userprivs->invisible && ~getRights()&OPERPRIVS_INVISIBLE) {
		send_numeric(441,"%s :They aren't on that channel",targetnick);
		return false;
	}
	opoverride = getRights()&OPERPRIVS_OPEROVERRIDE;
	if(!opoverride) {
		if(selfprivs == NULL) {
			send_numeric(442,"%s :You're not on that channel",params);
			return false;
		}
		if(userprivs->owner && !selfprivs->owner) {
			//:s 482 CHC #gsp!!test :You're not channel operator (Kick Owner - OWNER)
			send_numeric(482,"%s :You're not channel operator (Kick Owner - OWNER)",chan->getName());
			return false;
		}
		if(selfprivs->halfop == false && selfprivs->op == false && selfprivs->owner == false)  {
			//:s 482 Rawrarar #gsp!!test :You're not channel operator (Kick - HALFOPS)
			send_numeric(482,"%s :You're not channel operator (Kick - HALFOPS)",chan->getName());
			return false;
		}
		if(chan->onlyowner && !selfprivs->owner) {
			send_numeric(482,"%s :You're not channel operator (Kick - OWNER)",chan->getName());
			return false;
		}
	}
	char kickmsg[256];
	if(reason == NULL) {
		chan->sendMessage(userprivs->invisible,false,this,"KICK %s %s",chan->getName(),target->nick);
	} else {
		chan->sendMessage(userprivs->invisible,false,this,"KICK %s %s :%s",chan->getName(),target->nick,reason);
	}
	chan->removeUser(target,false,NULL);
	return true;
	notenoughparams:
	send_numeric(461,"KICK :Not enough parameters");
	return false;
}
bool Client::cmd_oper(char *params) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *email,*pass,*sendnoticetxt;
	char *query;
	int len;
	if(params == NULL) goto notenoughparams;
	email = strchr(params, ' ');
	if(email == NULL) goto notenoughparams;
	*email++=0;
	pass = strchr(email, ' ');
	if(pass == NULL) goto notenoughparams;
	*pass++=0;
	sendnoticetxt = strchr(pass,' ');
	int sendnotice;
	if(sendnoticetxt == NULL) {
		sendnotice = 1;
	} else {
		sendnotice = 0;
		*sendnoticetxt++=0;
	}
	mysql_real_escape_string(server.conn,email,email,strlen(email));
	mysql_real_escape_string(server.conn,pass,pass,strlen(pass));
	mysql_real_escape_string(server.conn,params,params,strlen(params));
	len = 1024 + strlen(pass) + strlen(email);
	query = (char *)malloc(len);
	snprintf(query,len,"CALL AuthClient(\"%s\",\"%s\",md5(\"%s\"),%d,%d)",params,email,pass,sd,sendnotice);
	mysql_ping(server.conn);
	if (mysql_query(server.conn, query)) {
	//todo: report error
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		free((void *)query);
		return false;
	}
	clearResults(server.conn);
	free((void *)query);
	return true;
	notenoughparams:
	send_numeric(461,"OPER :Not enough parameters");
	return false;
}
bool Client::cmd_login(char *params) {
//LOGIN 1 * c8837b23ff8aaa8a2dde915473ce0991 :CHCNiZ@chcniz@gmail.com
	MYSQL_RES *res;
	MYSQL_ROW row;
	int type;
	char *name,*pass,*loginstr;
	char *query = NULL;
	int len;
	char *email,*uniquenick;
	if(params == NULL) goto notenoughparams;
	name = strchr(params, ' ');//idk what the parameter is for
	if(name == NULL) goto notenoughparams;
	*name++=0;
	pass = strchr(name,' ');
	if(pass == NULL) goto notenoughparams;
	*pass++=0;
	loginstr = strchr(pass, ' ');
	if(loginstr == NULL) goto notenoughparams;
	*loginstr++=0;
	if(*loginstr == ':') loginstr++;
	type = atoi(params);
	len = 1024 + strlen(loginstr) + strlen(pass) + strlen(name);
	query = (char *)malloc(len);
	memset(query,0,len);
	email = strchr(loginstr,'@');
	if(email != NULL) *email++=0;

	if(email != NULL)
	mysql_real_escape_string(server.conn,email,email,strlen(email));
	mysql_real_escape_string(server.conn,pass,pass,strlen(pass));
	mysql_real_escape_string(server.conn,loginstr,loginstr,strlen(loginstr));
	if(email != NULL) {
		snprintf(query,len,"CALL AuthClient(\"%s\",\"%s\",\"%s\",%d,0)",loginstr,email,pass,sd);
	} else {
		snprintf(query,len,"CALL AuthClient(\"%s\",\"\",\"%s\",%d,0)",loginstr,pass,sd);
	}
	mysql_ping(server.conn);
	if (mysql_query(server.conn, query)) {
	//todo: report error
	        fprintf(stderr, "%s\n", mysql_error(server.conn));
		free((void *)query);
		return false;
	}
	clearResults(server.conn);
	free((void *)query);
	return true;
	notenoughparams:
	if(query != NULL) free((void *)query);
	send_numeric(461,"LOGIN :Not enough parameters");
	return false;
}
bool Client::cmd_invite(char *params) {
	Client *target;
	Channel *chan;
	char *channame;
	chanClient *info;
	if(params == NULL) goto notenoughparams;
	channame = strchr(params, ' ');
	if(channame == NULL) goto notenoughparams;
	*channame++ = 0;
	if(*channame == ':') {
		channame++;
	}
	target = find_user(params);
	chan = find_chan(channame);
	if(target == NULL || chan == NULL) {
		send_numeric(401, "%s :No such nick/channel",channame);
		return false;
	}
	info = chan->getUserInfo(this);
	if(chan->inviteonly == true) {
		if(info == NULL || info->halfop == false && info->op == false && info->owner == false) {
			send_numeric(482,"%s :You're not channel operator (Invite - HALFOPS)",chan->getName());
			return false;
		} else {
			addChannelInvite(target,chan);
		}
	}
	send_numeric(341,"%s %s",target->nick,chan->getName());
	target->messageSend(this,"INVITE %s %s",target->nick,chan->getName());
	return true;
	notenoughparams:
	send_numeric(461,"INVITE :Not enough parameters");
	return false;
}
bool Client::cmd_ison(char *params) {
	Client *target;
	char *seehost;
	char *star = "*\0";
	if(params == NULL) goto notenoughparams;
	strip(params, ' ');
	target = find_user(params);
	if(target == NULL || (target->getRights() & OPERPRIVS_HIDDEN && ~this->getRights() & OPERPRIVS_SEEHIDDEN)) {
		send_numeric(401,"%s :No such nick/channel",params);
		return false;
	}
	if(canSeeIp(this,target)) {
		seehost = target->host;
	} else seehost = star;
	send_numeric(303,":%s!%s@%s",target->nick,target->user,seehost);
	return true;
	notenoughparams:
	send_numeric(461,"ISON :Not enough parameters");
	return false;
}
bool Client::cmd_away(char *params) {
	char *text;
	if(params == NULL) {
		awaytext[0] = 0;
	} else {
		text = strchr(params, ':');
		if(text == NULL) text = params;
		else text++;
		snprintf(awaytext,sizeof(awaytext),"%s",text);
	}
	return true;
}
bool Client::cmd_lusers(char *params) {
	std::list<Client *>::iterator iterator;
	iterator=server.client_list.begin();
	Client *c;
	int numopers = 0,numusers = 0;
	while(iterator != server.client_list.end()) {
		c=*iterator;
		numusers++;
		if((c->getRights() != 0)) {
			numopers++;
		}
		iterator++;
	}
	send_numeric(251,":There are %d users",numusers);
	send_numeric(252,":%d IRC Operators online",numopers);
	return true;
}
Client::Client(clientParams *params) {
	sd=params->sd;
	u_long on=1;
	memset(&nick,0,sizeof(nick));
	memset(&user,0,sizeof(user));
 	memset(&host,0,sizeof(host));
	memset(&realname,0,sizeof(realname));
	memset(&quitreason,0,sizeof(quitreason));
	memcpy(&peer,&(params->peer),sizeof(struct sockaddr_in));
	rightsmask = 0;
	uniquenick[0] = 0;
	toquit=false;
	waiting_ping=false;
	modeflags = 0;
	sprintf_s(host,sizeof(host),"%s",inet_ntoa(peer.sin_addr));
	len=0;
	awaytext[0] = 0;
	profileid = 0;
	welcomed=false;
	gagged = false;
	lastWeightReduction = time(NULL);
	gameid = 0;
	registered = 0;
	weight = 0;
	connected=time(NULL);
	encryted = false;
	last_reply=connected+(PINGTIME*4);
	free((void *)params);
 }
 void Client::mainLoop(fd_set *rset) {
	if(checkQuit()) { //check if theres a reason to disconnect them
		goto end;
	}
	if(!FD_ISSET(sd,rset)) return;
	memset(&buff,0,sizeof(buff));
	len = recv(sd,buff,MAX_BUFF,0);
	if(len == 0 ) quitUser("Client Exited");
	if(len<0) return;
	if(!do_db_check()) {
		send_numeric(400,"Lost Database connection - Command Dropped.");
		return;
	}
	if(encryted == true) {
		gs_peerchat(&eClient,(unsigned char *)&buff,len);
	}
	len = makeStringSafe((char *)&buff, sizeof(buff));
	reduceWeight();
	parseIncoming();
	return;
end:
	 deleteClient(this);
 }
void Client::reduceWeight() {
	time_t timenow = time(NULL);
	if(timenow > lastWeightReduction) {
		int reduction = timenow - lastWeightReduction;
		reduction *= 5;
		weight -= reduction;
		if(weight < 0) weight = 0;
		lastWeightReduction = time(NULL);
	}
}
 void Client::parseIncoming(char *buff, int len) {
	 char tbuff[MAX_BUFF];
	 bool found=false;
	 bool (Client::*cFunc)(char *);
	 char *data;
	 memcpy(&tbuff,buff,sizeof(tbuff));
	 data=find_and_cut(tbuff,0,' ');
	 if(data==NULL) {
		 data=(char *)&tbuff;
	 }
	commandInfo cmds[] = {{"USER",OPERPRIVS_NONE,CMDREGISTER_NO,0,&Client::cmd_user},
		{"NICK",OPERPRIVS_NONE,CMDREGISTER_EITHER,75,&Client::cmd_nick},
		{"WALLOPS",OPERPRIVS_WALLOPS,CMDREGISTER_YES,0,&Client::cmd_wallops},
		{"SETRIGHTS",OPERPRIVS_SERVMANAGE,CMDREGISTER_YES,0,&Client::cmd_setrights},
		{"OPERMSG",OPERPRIVS_WALLOPS,CMDREGISTER_YES,0,&Client::cmd_opermsg},
		{"LISTOPERS",OPERPRIVS_LISTOPERS,CMDREGISTER_YES,0,&Client::cmd_listopers},
		{"LISTUSERS",OPERPRIVS_LISTOPERS,CMDREGISTER_YES,0,&Client::cmd_listusers},
		{"QUIT",OPERPRIVS_NONE,CMDREGISTER_EITHER,0,&Client::cmd_quit},
		{"PING",OPERPRIVS_NONE,CMDREGISTER_EITHER,0,&Client::cmd_pingme},
		{"PONG",OPERPRIVS_NONE,CMDREGISTER_EITHER,0,&Client::cmd_pong},
		{"JOIN",OPERPRIVS_NONE,CMDREGISTER_YES,5,&Client::cmd_join},
		{"PART",OPERPRIVS_NONE,CMDREGISTER_YES,5,&Client::cmd_part},
		{"NAMES",OPERPRIVS_NONE,CMDREGISTER_YES,10,&Client::cmd_names},
		{"MODE",OPERPRIVS_NONE,CMDREGISTER_YES,3,&Client::cmd_mode},
		{"PRIVMSG",OPERPRIVS_NONE,CMDREGISTER_YES,10,&Client::cmd_privmsg},
		{"NOTICE",OPERPRIVS_NONE,CMDREGISTER_YES,10,&Client::cmd_notice},
		{"ATM",OPERPRIVS_CTCP,CMDREGISTER_YES,0,&Client::cmd_atm},
		{"UTM",OPERPRIVS_NONE,CMDREGISTER_YES,0,&Client::cmd_utm},
		{"TOPIC",OPERPRIVS_NONE,CMDREGISTER_YES,5,&Client::cmd_topic},
		{"WHOIS",OPERPRIVS_NONE,CMDREGISTER_YES,10,&Client::cmd_whois},
		{"SETUSERMODE", OPERPRIVS_NONE, CMDREGISTER_YES,3,&Client::cmd_setusermode},
		{"LISTUSERMODES", OPERPRIVS_NONE, CMDREGISTER_YES,3,&Client::cmd_listusermodes},
		{"DELUSERMODE", OPERPRIVS_NONE, CMDREGISTER_YES, 3, &Client::cmd_delusermode},
		{"KILL", OPERPRIVS_KILL, CMDREGISTER_YES, 3, &Client::cmd_kill},
		{"KLINE", OPERPRIVS_KILL, CMDREGISTER_YES, 3, &Client::cmd_kline},
		{"USERHOST", OPERPRIVS_NONE, CMDREGISTER_EITHER, 5, &Client::cmd_userhost},
		{"USRIP", OPERPRIVS_NONE, CMDREGISTER_EITHER, 5, &Client::cmd_userhost},
		{"SETCHANPROPS", OPERPRIVS_NONE, CMDREGISTER_YES, 3, &Client::cmd_setchanprops},
		{"LISTCHANPROPS", OPERPRIVS_NONE, CMDREGISTER_YES, 3, &Client::cmd_listchanprops},
		{"DELCHANPROPS", OPERPRIVS_NONE, CMDREGISTER_YES, 3, &Client::cmd_delchanprops},
		{"SETGROUP", OPERPRIVS_NONE, CMDREGISTER_YES, 5, &Client::cmd_setgroup},
		{"LIST", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_listchans},
		{"WHOWAS", OPERPRIVS_CTCP, CMDREGISTER_YES, 3, &Client::cmd_whowas},
		{"GETKEY", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_getkey},
		{"SETKEY", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_setkey},
		{"SETCHANKEY", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_setchankey},
		{"GETCHANKEY", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_getchankey},
		{"SETCKEY", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_setckey},
		{"GETCKEY", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_getckey},
//		{"MATCH", OPERPRIVS_NONE, CMDREGISTER_YES, 0, &Client::cmd_match},
		{"KICK", OPERPRIVS_NONE, CMDREGISTER_YES, 5, &Client::cmd_kick},
		{"WHO", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_who},
		{"CDKEY", OPERPRIVS_NONE, CMDREGISTER_EITHER, 0, &Client::cmd_cdkey},
		{"CRYPT", OPERPRIVS_NONE, CMDREGISTER_NO, 0, &Client::cmd_crypt},
		{"OPER", OPERPRIVS_NONE, CMDREGISTER_YES, 75, &Client::cmd_oper},
		{"INVITE", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_invite},
		{"ISON", OPERPRIVS_NONE, CMDREGISTER_YES, 10, &Client::cmd_ison},
//		{"REHASH", OPERPRIVS_SERVMANAGE, CMDREGISTER_YES, 75, &Client::cmd_ison},
		{"AWAY", OPERPRIVS_NONE, CMDREGISTER_YES, 5, &Client::cmd_away},
		{"LOGIN", OPERPRIVS_NONE, CMDREGISTER_NO, 75, &Client::cmd_login},
		{"FJOIN", OPERPRIVS_MANIPULATE, CMDREGISTER_YES, 5, &Client::cmd_fjoin},
		{"LUSERS", OPERPRIVS_LISTOPERS, CMDREGISTER_YES, 5, &Client::cmd_lusers},
	};
	for(int i=0;i<(sizeof(cmds)/sizeof(commandInfo));i++) {
		if(!stricmp(cmds[i].name,tbuff)) { //used to be strnicmp but removed because of issues with user and userhost, why was it like that?
			found=true;
			memcpy(&tbuff,buff,sizeof(tbuff));
			strip((char *)&tbuff,'\r');
			strip((char *)&tbuff,'\n');

			data=find_first((char *)&tbuff,' ',strlen(tbuff));
			if(data != NULL) data++;
			cFunc=cmds[i].mpFunc;
			if(cmds[i].registered==CMDREGISTER_NO && registered==true) {
				send_numeric(462,":Unauthorized command (already registered)");
				break;
			} else if(cmds[i].registered==CMDREGISTER_YES && registered==false) {
				send_numeric(451,":You have not registered");
				break;
			}
			if(cmds[i].rightsmask != OPERPRIVS_NONE) { //privileged command
				if(!(rightsmask & cmds[i].rightsmask)) { 
					found=false; //pretend the command doesn't exist
					break;
				}
			}
			if(~getRights() & OPERPRIVS_FLOODEXCEMPT)
				weight += cmds[i].flood;
			if(weight > 600) {
				quitUser("Excess Flood"); 
				return;
			}
			else if(weight > 300) {
	 			sendToClient(":SERVER!SERVER@* PRIVMSG %s :Excess Flood: %d",nick,weight);
				if(cmds[i].flood > 5) break;
			}
			(*this.*cFunc)(data);
			break;
		}
	}
	if(!found) {
		strip((char *)&tbuff,' ');
		send_numeric(421,"%s :Unknown command",tbuff);
		if(~getRights() & OPERPRIVS_FLOODEXCEMPT)
			weight += 5;
	}
 }
 int Client::sendToClient(char *fmt, ...) {
	if(modeflags & EUserMode_Quiet) return 0;
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	if(slen>=MAX_BUFF-2) return -1;
	sbuff[slen]='\r';
	sbuff[slen+1]='\n';
	slen+=2;
	if(encryted == true) {
		gs_peerchat(&eServer,(unsigned char *)&sbuff,slen);
	}
	return send(sd,sbuff,slen,MSG_NOSIGNAL);
 }
 int Client::sendToClientQuiet(char *fmt, ...) {
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	if(slen>=MAX_BUFF-2) return -1;
	sbuff[slen]='\r';
	sbuff[slen+1]='\n';
	slen+=2;
	if(encryted == true) {
		gs_peerchat(&eServer,(unsigned char *)&sbuff,slen);
	}
	return send(sd,sbuff,slen,MSG_NOSIGNAL);
 }
 void Client::messageSend(Client *from, char *fmt, ...) {
	char *unick,*uuser,*uhost;
	char star[] = {'*', '\0'};
	char sbuff[MAX_BUFF];
	int slen;
	va_list args;
	memset(&sbuff,0,sizeof(sbuff));
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	from->getUserInfo(&unick,&uuser,&uhost,NULL);
	if(!canSeeIp(this,from)) {
		uhost = (char *)&star;
	}
	sendToClient(":%s!%s@%s %s",unick,uuser,uhost,sbuff);
	return;
 }
 void Client::send_numeric(short num, char *fmt, ...) {
	char sbuff[MAX_BUFF],sbuff2[MAX_BUFF];
	int slen,slen2;
	memset(&sbuff,0,sizeof(sbuff));
	memset(&sbuff2,0,sizeof(sbuff2));
	va_list args;
	va_start( args, fmt );	
	slen = vsprintf_s(sbuff,sizeof(sbuff),fmt,args);
	va_end( args );
	if(nick[0] != 0) {
	slen2 = sprintf_s(sbuff2,sizeof(sbuff2),":s %03i %s %s",num,nick,sbuff);
	} else {
		slen2 = sprintf_s(sbuff2,sizeof(sbuff2),":s %03i * %s",num,sbuff);
	}
	//:s 421 CHC raw :Unknown command
	sendToClient(sbuff2);
 }
 void Client::parseIncoming() {
	//this function might need to be rewritten
	last_reply = time(NULL);
	char *p = (char *)&buff;	
	char *x;
	while(true) {
		x = p;
		p = strstr(p,"\r\n");
		if(p == NULL) { break; }
		*p = 0;
		p+=2;
		parseIncoming(x,(int)(x - ((char *)&buff)));
	}
	if((x - ((char *)&buff)) > 2 && *x != 0) {
		parseIncoming(x,(int)(x - ((char *)&buff)));
	} else if(p == NULL && x == (char *)&buff) {
		char cmd[MAX_BUFF];
		memcpy(&cmd,&buff,sizeof(cmd));
		p = strtok (cmd,"\r\n");
		while (p != NULL)
		{
			parseIncoming(p,strlen(p)+2);
			p = strtok (NULL, "\r\n");
		}
	}
}
 void Client::sendWelcome() {
	 if(welcomed) return;
	 if(isGlobalBanned(this) || numUsersByIP(getIP()) > 5) {
		 if(!isGlobalExcempt(this)) {
			 toquit = true;
			 return;
		 }
	 }
	 send_numeric(1,":Welcome to the Matrix %s",nick);
	 send_numeric(2,":Your host is xs0, running version 1.0");
	 send_numeric(3,":This server was created Fri Oct 19 1979 at 21:50:00 PDT");
	 send_numeric(4,"s 1.0 iq biklmnopqustvhe");
	 send_numeric(375,":- (M) Message of the day - ");
	 send_numeric(372,":- Welcome to GameSpy");
	 send_numeric(376,":End of MOTD command");
	 sendToAllWithMode((int)EUserMode_ShowConns,":SERVER!SERVER@* NOTICE :NEW USER: %s!%s@%s",nick,user,host);
	 applyUserModes(this);
	 welcomed=true;
 }
 void Client::getUserInfo(char **nick, char **user, char **host, char **realname) {
	 if(nick != NULL) {
		 *nick=(char *)&this->nick;
	 }
	 if(user != NULL) {
		 *user=(char *)&this->user;
	 }
	 if(host != NULL) {
		 *host=(char *)&this->host;
	 }
	 if(realname != NULL) {
		 *realname=(char *)&this->realname;
	 }
 }
 uint32_t Client::getRights() {
	 return rightsmask;
 }
 void Client::sendUserInfo(Client *target, char *cmd) {
	 struct tm * timeinfo;
	 timeinfo = localtime ( &connected );
	 char timestr[256];
	 strftime(timestr,sizeof(timestr),"%m/%d/%Y %H:%M",timeinfo);
	 sendToClient(":SERVER!SERVER@* PRIVMSG %s :%s \\name\\%s\\user\\%s\\realname\\%s\\hostmask\\%s\\rightsmask\\%i\\connected\\%s",nick,cmd,target->nick,target->user,target->realname,target->host,target->rightsmask,timestr);
	 return;
 }
bool Client::isGagged() {
	return gagged;
}
void Client::togGag() {
	gagged = !gagged;
}
void Client::setGagged(bool value) {
	gagged = value;
}
 void Client::quitUser(char *reason) {
	 char *p=reason;
	 if(p[0]==':') p++;
	 //memset(&quitreason,0,sizeof(quitreason));
	 //sprintf_s(quitreason,sizeof(reason),"%s",reason);
	 //sendOnceAllChan(Client *source, bool sendtosource,char *str,...)
	 sendOnceAllChan(this,false,"QUIT :%s",p);
	 removeFromAllChans(this);
	 addWhowas(this);
	 sendToClient("ERROR :Closing Link: s (%s)",p);
	 sendToAllWithMode((int)EUserMode_ShowConns,":SERVER!SERVER@* NOTICE :USER QUIT: %s!%s@%s (%s)",nick,user,host,p);
	 toquit = true;
 }
 bool Client::checkQuit() {
	 //if(toquit)
		//sendToClient("ERROR :Closing Link: s (%s)",quitreason);
	 return toquit;
 }
 int Client::getProfileID() {
	 return profileid;
 }
 time_t Client::getLastreply() {
	 return last_reply;
 }
 Client::~Client() {
	std::list<customKey *>::iterator iterator = userKeys.begin();
	customKey *key;
	while(iterator != userKeys.end()) {
		key = *iterator;
		free((void *)key); //dw about removing from list since its going to be freed anyways
		iterator++;
	}
	removeChannelInvite(this,NULL);
	close(sd);
 }
 time_t Client::getConnectedTime() {
	 return connected;
 }
 bool Client::getUserKey(char *name, char *dst,int len) {
	 memset(dst,0,len);
	 if(stricmp(name,"username") == 0) {
		 strncpy(dst,user,strlen(user)%len);
		 return true;
	 }
	 if(stricmp(name,"realname") == 0) {
		 strncpy(dst,realname,strlen(realname)%len);
		 return true;
	 }
	 if(stricmp(name,"profileid") == 0) {
		 snprintf(dst,len,"%d",profileid);
		 return true;
	 }
	 if(stricmp(name,"gameid") == 0) {
		 snprintf(dst,len,"%d",gameid);
		 return true;
	 }
	 if(stricmp(name,"socket") == 0) {
		 snprintf(dst,len,"%d",sd);
		 return true;
	 }
	 if(stricmp(name,"uniquenick") == 0) {
		 strncpy(dst,uniquenick,strlen(uniquenick)%len);
		 return true;
	 }
	 return false;
 }
bool Client::addUserParam(customKey* key) {
	customKey *curkey = getParamValue(key->name,NULL,0);
	if(curkey != NULL) {
		if(key->value[0] == 0) {
			userKeys.remove(curkey);
			return false;
		}
		strcpy(curkey->value,key->value);
		return false;
	}
	customKey *newkey = (customKey *)malloc(sizeof(customKey));
	if(newkey != NULL) {
		memcpy(newkey,key,sizeof(customKey));
		userKeys.push_back(newkey);
	} else return false;
	return true;
}
customKey *Client::getParamValue(char *name, char *dst, int dstlen) {
	std::list<customKey *>::iterator iterator = userKeys.begin();
	customKey *key;
	while(iterator != userKeys.end()) {
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
int Client::getSocket() {
	return sd;
}
void Client::logUserIn(int userid, int profileid, bool sendnotice) {
	operInfo *oper = getOperInfo(profileid);
	if(userid == 0) {
		//TODO: maybe spam stuff?
		send_numeric(708, ":Invalid login information (%d)",profileid);
		return;
	}
	this->profileid = profileid;
	if(oper != NULL) {
		this->rightsmask = oper->rightsmask;
	} else {
		this->rightsmask = 0;
	}
	send_numeric(707,"%d %d",userid, profileid);
	if(sendnotice) {
		sendToClient(":SERVER!SERVER@* NOTICE %s :Authenticated",nick);
		if(rightsmask != 0) {
			sendToClient(":SERVER!SERVER@* NOTICE %s :Rights Granted",nick);
		}
	} 
	if(rightsmask != 0) {
		sendToAllWithMode((int)EUserMode_ShowConns,":SERVER!SERVER@* NOTICE :%s now has rightsmask: %i(0x%08x)",nick,rightsmask,rightsmask);
	}
	return;
}
int Client::getModeFlags() {
	return modeflags;
}
int Client::getGameID() {
	return gameid;
}
uint32_t Client::getIP() {
	return peer.sin_addr.s_addr;
}
