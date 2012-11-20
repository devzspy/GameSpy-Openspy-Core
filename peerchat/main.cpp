#include "main.h"
#include "server.h"
#include "channel.h"
peerchatServer server;
modLoadOptions *modoptions;
modInfo moduleInfo = {"Peerchat","Gamespy Matrix Clone"};
bool do_db_check();
void checkPing(Client *c) {
	int now = (int)time(NULL);
	int then = (int)c->getConnectedTime();
	int ping_time=(int)c->getLastreply();
	if(!(then+15>now)) {
		if(!((ping_time+PINGTIME)>now)) {
			if(!c->waiting_ping) {
				c->sendToClientQuiet("PING :s");
				c->waiting_ping=true;
			}
		}
		if(!(c->getLastreply()+(PINGTIME*2)>now) && c->waiting_ping==true) {
			c->quitUser("Ping Timeout");
		}
	}
}
void processClients(fd_set *rset) {
	Client *c;
	std::list <Client *> clist = server.client_list;
	std::list<Client *>::iterator iterator=clist.begin();
		 while(iterator != clist.end()) {
			c=*iterator;
			checkPing(c);
			c->mainLoop(rset);
			iterator++;
		 }
}
int getnfds(fd_set *rset) {
	int hsock = 0;
	Client *c;
	std::list <Client *> clist = server.client_list;
	std::list<Client *>::iterator iterator=clist.begin();
		 while(iterator != clist.end()) {
			c=*iterator;
			int sock = c->getSocket();
			if(sock > hsock) hsock = sock;
			FD_SET(sock,rset);
			iterator++;
		 }
	return hsock;
}
void loadChanProps() {
	MYSQL_RES *res;
	MYSQL_ROW row;
   if (mysql_query(server.conn, "SELECT `chankey`,`chanmask`,`comment`,`entrymsg`,Unix_Timestamp(`expires`),`groupname`,`limit`,`mode`,`onlyowner`,`setbynick`,`setbypid`,Unix_Timestamp(`setondate`),`topic`,`setbyhost` FROM `chanprops`")) {
      fprintf(stderr, "%s\n", mysql_error(server.conn));
      exit(1);
   }
   chanProps *props;
   res = mysql_store_result(server.conn);
   while ((row = mysql_fetch_row(res)) != NULL) {
	props = (chanProps *)malloc(sizeof(chanProps));
	memset(props,0,sizeof(chanProps));
	strcpy(props->chankey,row[0]);
	strcpy(props->chanmask,row[1]);
	strcpy(props->comment,row[2]);
	strcpy(props->entrymsg,row[3]);
	props->expires = atoi(row[4]);
	strcpy(props->groupname,row[5]);
	props->limit = atoi(row[6]);
	strcpy(props->modes,row[7]);
	props->onlyowner = atoi(row[8]);
	if(row[9][0] != 0) {
		strcpy(props->setbynick,row[9]);
	} else {
		strcpy(props->setbynick,"SERVER");
	}
	props->setbypid = atoi(row[10]);
	props->setondate = atoi(row[11]);
	strcpy(props->topic,row[12]);
	strcpy(props->setbyhost,row[13]);
	server.chanprops_list.push_back(props);
	applyChanProps(props, false);
   }
}
void loadUserModes() {
	MYSQL_RES *res;
	MYSQL_ROW row;
   if (mysql_query(server.conn, "SELECT `chanmask`,`comment`,Unix_Timestamp(`expires`),`hostmask`,`machineid`,`modeflags`,`profileid`,`setbyhost`,`setbynick`,`setbypid`,Unix_Timestamp(`setondate`),`usermodeid` FROM `chanusermodes`")) {
      fprintf(stderr, "%s\n", mysql_error(server.conn));
      exit(1);
   }
   userMode *usermode;
   res = mysql_store_result(server.conn);
   while ((row = mysql_fetch_row(res)) != NULL) {
	usermode = (userMode *)malloc(sizeof(userMode));
	memset(usermode,0,sizeof(userMode));
	strcpy(usermode->chanmask,row[0]);
	strcpy(usermode->comment,row[1]);
	usermode->expires = atoi(row[2]);
	strcpy(usermode->hostmask,row[3]);
	strcpy(usermode->machineid,row[4]);
	usermode->modeflags = atoi(row[5]);
	usermode->profileid = atoi(row[6]);
	strcpy(usermode->setbyhost,row[7]);
	strcpy(usermode->setbynick,row[8]);
	usermode->setbypid = atoi(row[9]);
	usermode->setondate = atoi(row[10]);
	usermode->usermodeid  = atoi(row[11]);
	usermode->isGlobal = 1;
	server.usermodes_list.push_back(usermode);
	applyUserMode(usermode);
   }
   mysql_free_result(res);
}
void loadChanClients() {
	MYSQL_RES *res;
	MYSQL_ROW row;
   if (mysql_query(server.conn, "SELECT `chanmask`,`gameid` FROM `chanclients`")) {
      fprintf(stderr, "%s\n", mysql_error(server.conn));
      exit(1);
   }
   chanGameClient *client;
   res = mysql_store_result(server.conn);
   while ((row = mysql_fetch_row(res)) != NULL) {
	client = (chanGameClient *)malloc(sizeof(chanGameClient));
	memset(client,0,sizeof(chanGameClient));
	strncpy(client->chanmask,row[0],strlen(row[0])%sizeof(client->chanmask));
	client->gameid = atoi(row[1]);
	server.chanclient_list.push_back(client);
   }
   mysql_free_result(res);
}
int setupQueueListener() {
	int sd;
	struct sockaddr_in si_me;
	if((sd=socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return -1;
	memset((char *)&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(QUEUEPORT);
	uint32_t bindip = modoptions->getConfInt(modoptions->moduleArray,"queueip");
	if(bindip == 0) {
		si_me.sin_addr.s_addr = htonl(0x7F000001);
	} else si_me.sin_addr.s_addr = bindip;
	if(bind(sd,(struct sockaddr *)&si_me,sizeof(si_me)) == -1)
		return -1;
	return sd;
}
void processQueue(int sd) {
	//notice: its trusted that the source of this data is actually from within the system and trustworthy, things like buffer overflows arn't really checked too tightly
	ssize_t len;
	char buf[2048];
	struct sockaddr si_other;
	socklen_t slen;
	msgAddOper *addOpermsg;
	msgDelOper *delOpermsg;
	msgSetChanProps *setChanPropsmsg;
	msgDelChanProps *delChanPropsmsg;
	msgAuthClient *authmsg;
	userMode *setUsermodemsg;
	userMode *um;
	chanProps *props;
	chanGameClient *gameclient,*ngameclient;
	Client *c;
	int *usermodeid;
	memset((char *)&buf,0, sizeof(buf));
	len = recvfrom(sd,(char *)&buf,sizeof(buf),0, (struct sockaddr *)&si_other, &slen);
	if(len == 0) return;
	switch(buf[0]) {
		case EQueueID_AddOper:
			addOpermsg = (msgAddOper *)(buf+1);
			addOper(addOpermsg->profileid,addOpermsg->rightsmask);
			return;
		case EQueueID_DelOper:
			delOpermsg = (msgDelOper*)(buf+1);
			delOper(delOpermsg->profileid);
			return;
		case EQueueID_SetUserMode:
			setUsermodemsg = (userMode*)(buf+1);
			um = (userMode *)malloc(sizeof(userMode));
			if(um == NULL) return;
			memcpy(um,setUsermodemsg,sizeof(userMode));
			if(getUserMode(um->usermodeid) == NULL) {
				server.usermodes_list.push_back(um);
				applyUserMode(um);
			}
			return;
		case EQueueID_DelUserMode: 
			usermodeid = (int *)(buf+1);
			um = getUserMode(*usermodeid);
			if(um != NULL) {
				removeUsermode(um,true);
			}
		return;
		case EQueueID_SetChanProps:
			setChanPropsmsg = (msgSetChanProps *)(buf+1);			
			props = findChanProp(setChanPropsmsg->props.chanmask);
			if(props == NULL) {
				props = (chanProps *)malloc(sizeof(chanProps));
				if(props == NULL) return;
				memcpy(props,&setChanPropsmsg->props,sizeof(chanProps));
				server.chanprops_list.push_back(props);
			} else {
				memcpy(props,&setChanPropsmsg->props,sizeof(chanProps));
			}
			applyChanProps(props, setChanPropsmsg->kickexisting);
		return;
		case EQueueID_DelChanProps:
			delChanPropsmsg = (msgDelChanProps *)(buf+1);
			props = findChanProp(delChanPropsmsg->chanmask);
			if(props != NULL) {
				deleteChanProp(props, delChanPropsmsg->kickexisting);
			}
		case EQueueID_AddChanClient: {
			gameclient = (chanGameClient *)(buf+1);
			ngameclient = (chanGameClient *)malloc(sizeof(chanGameClient));
			memcpy(ngameclient,gameclient,sizeof(chanGameClient));
			server.chanclient_list.push_back(ngameclient);			
			return;
		}
		case EQueueID_DelChanClient: {
			gameclient = (chanGameClient *)(buf+1);
			deleteGameClient(gameclient->chanmask, gameclient->gameid);
			return;
		}
		case EQueueID_AuthClient: {
			authmsg = (msgAuthClient *)(buf+1);
			c = findClientBySocket(authmsg->sd);
			if(c != NULL) {
				c->logUserIn(authmsg->userid,authmsg->profileid,authmsg->sendnotice==1?true:false);
				strcpy(c->uniquenick,authmsg->uniquenick);
			}
		}
		return;
	}
}
void *openspy_mod_run(modLoadOptions *options)
{
#ifdef WIN32
	WSADATA ws;
	WSAStartup(MAKEWORD(1,0),&ws);
#endif
	int on=1,sda;
#ifdef WIN32
	int psz;
#else
	socklen_t psz;
#endif
	u_long on_a=1;
	struct  sockaddr_in peer;
	int sdl,sdq;
	fd_set  rset;
	MYSQL_RES *res;
	MYSQL_ROW row;
    modoptions = options;
    peer.sin_addr.s_addr = options->bindIP;
    peer.sin_port        = htons(6667);
    peer.sin_family      = AF_INET;
    server.conn = mysql_init(NULL);
    mysql_options(server.conn,MYSQL_OPT_RECONNECT, (char *)&on_a);
   /* Connect to database */


   if (!mysql_real_connect(server.conn, options->mysql_server,
         options->mysql_user, options->mysql_password, options->mysql_database, 0, NULL, CLIENT_MULTI_RESULTS)) {
           fprintf(stderr, "%s\n", mysql_error(server.conn));
           return NULL;
   }
   server.games = options->games;
   server.num_games = options->totalgames;
   loadChanProps();
   loadUserModes();
   loadChanClients();
   if (mysql_query(server.conn, "SELECT `profileid`,`rightsmask` FROM `Matrix`.`globalopers`")) {
      fprintf(stderr, "%s\n", mysql_error(server.conn));
      exit(1);
   }
   res = mysql_store_result(server.conn);
   while ((row = mysql_fetch_row(res)) != NULL) {
	addOper(atoi(row[0]),atoi(row[1]));
   } 
    mysql_free_result(res);
    sdl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sdl < 0) return NULL;
    if(setsockopt(sdl, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))
      < 0) return NULL;
    if(bind(sdl, (struct sockaddr *)&peer, sizeof(struct sockaddr_in))
      < 0) return NULL;
    if(listen(sdl, SOMAXCONN)
      < 0) return NULL;
    sdq = setupQueueListener();
    struct timeval timeout;
    memset(&timeout,0,sizeof(struct timeval));
    int hsock;
    for(;;) {
	timeout.tv_sec = PINGTIME;
	FD_ZERO(&rset);
	FD_SET(sdl, &rset);
	FD_SET(sdq, &rset);
	hsock = getnfds(&rset);
	if(hsock < sdl) hsock = sdl;
	if(hsock < sdq) hsock = sdq;
	Client *c=NULL;
	clientParams *param;

	if(select(hsock+1, &rset, NULL, NULL, &timeout)
          < 0) continue;
        if(FD_ISSET(sdl, &rset)) {
	} else if(FD_ISSET(sdq,&rset)) {
		processQueue(sdq);
		continue;
	} else {
		processClients(&rset);
		checkExpiry();
		continue;
	}
        psz = sizeof(struct sockaddr_in);
        sda = accept(sdl, (struct sockaddr *)&peer, &psz);
        if(sda <= 0) continue;
		if(!do_db_check()) {
			const char *errmsg = "ERROR :Closing Link: s (DATABASE ERROR - TRY AGAIN LATER)\r\n";
			send(sda,(const char *)errmsg,strlen(errmsg),MSG_NOSIGNAL);
			close(sda);
			continue; //TODO: send database error message
		}
		param = (clientParams *)malloc(sizeof(clientParams)); //its up to the thread to free this
		param->sd=sda;
		param->server=&server;
		memcpy(&param->peer,&peer,sizeof(struct sockaddr));
		c=new Client(param);
		server.client_list.push_back(c);
    }
   return NULL;
}
modInfo *openspy_modInfo() {
	return &moduleInfo;
}
bool openspy_mod_query(char *sendmodule, void *data,int len) {
	peerchatMsgData *msg = (peerchatMsgData *)data;
	msgNumUsersOnChan *numUsersMsg;
	Channel *c;
	do_db_check(); //since the only message doesn't do a query, don't worry about returning if no mysql server yet
	switch(msg->msgid) {
		case EMsgID_NumUsersOnChan:
			numUsersMsg = (msgNumUsersOnChan *)msg->data;
			c = find_chan(numUsersMsg->channelName);
			if(c != NULL) {
				numUsersMsg->numusers = c->getNumUsers(numUsersMsg->showInvisible);
			}	
			return true;
		break;
		default:
//		printf("unknown message\n"); //TODO: make send module log message
		return false;
	}
}

bool do_db_check() {
	int mysql_status = mysql_ping(server.conn);
	switch(mysql_status) {
		case CR_COMMANDS_OUT_OF_SYNC: {
			modoptions->logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server out of sync\n");
			break;
		}
		case CR_SERVER_GONE_ERROR: {
			modoptions->logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server has gone away\n");
			break;
		}
		case CR_UNKNOWN_ERROR: {
			modoptions->logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server has an unknown connection error\n");
			break;
		}
	}
	mysql_status = mysql_ping(server.conn); //check if reconnect was successful
	if(mysql_status != 0) {
		modoptions->logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server reconnect was unsuccessful\n");
	}
	return mysql_status == 0;
}
