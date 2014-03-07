//player searching, gpsp.gamespy.com port 29901
#include "main.h"
MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;
modInfo moduleInfo = {"playersearch","Gamespy Player Search(GPSP)"};
modLoadOptions servoptions;
bool do_db_check();
#ifdef _WIN32
DWORD
#else
pthread_t
#endif
handleConnection(int sd) {
#ifdef _WIN32
	HANDLE threadHandle;
	DWORD tid;
#else
	pthread_t tid;
	pthread_attr_t  attr;
	pthread_attr_init(&attr);
#endif
	threadOptions *options;
	options = (threadOptions *)malloc(sizeof(threadOptions));
	//memset(options,0,sizeof(threadOptions));
	options->sd = sd;
#ifdef _WIN32
	threadHandle = CreateThread(0,0,(LPTHREAD_START_ROUTINE)processConnection,options,0,&tid);
#else
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(pthread_create(&tid, &attr, (void *(*)(void *))processConnection, (void *)options)) 
		return(0);
#endif
	return tid;
}
void *processConnection(threadOptions *options) {
	char buf[2048];
	char type[128];
	int len;
	int sd = options->sd;
	free((void *)options);
	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv);
	len = recv(sd,buf,sizeof(buf),MSG_NOSIGNAL);
	if(len == -1) { //timeout
		sendError(sd,"The search has timedout");
		return NULL;
	}
	makeStringSafe((char *)&buf, sizeof(buf));
	if(!find_param(0, buf, type,sizeof(type))) {
		sendError(sd,"Error recieving request");
		return NULL;	
	}
	//TODO: pmatch(product matching), others(showing buddies),otherslist(wait until GPCM is implemented)
	if(stricmp(type,"valid") == 0) {
		checkEmailValid(sd,buf);
	} else if(stricmp(type,"nicks") == 0) {
		sendNicks(sd,buf);
	} else if(stricmp(type,"check") == 0) {
		checkNick(sd,buf);
	} else if(stricmp(type,"newuser") == 0) {
		newUser(sd,buf);
	} else if(stricmp(type,"search") == 0) { 
		searchUsers(sd,buf);
	} else if(stricmp(type,"others") == 0) {
		sendReverseBuddies(sd,buf);
	} else if(stricmp(type,"otherslist") == 0) {
		sendReverseBuddies(sd,buf);
	} else if(stricmp(type,"uniquesearch") == 0) { //nameinator
	} else if(stricmp(type,"profilelist") == 0) { //nameinator
	}else {
		sendError(sd,"Error recieving request");	
		return NULL;
	}
	close(sd);
	return NULL;
}
void *openspy_mod_run(modLoadOptions *options)
{
  int sda,sd;
  socklen_t psz;
  struct  sockaddr_in peer;
  memset(&peer,0,sizeof(peer));
  int on=1;
  memcpy(&servoptions,options,sizeof(modLoadOptions));
  conn = mysql_init(NULL);
  mysql_options(conn,MYSQL_OPT_RECONNECT, (char *)&on);
  /* Connect to database */
  if (!mysql_real_connect(conn, servoptions.mysql_server,
      servoptions.mysql_user, servoptions.mysql_password, servoptions.mysql_database, 0, NULL, CLIENT_MULTI_RESULTS)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      return NULL;
  }
  peer.sin_port        = htons(SEARCHPORT);
  peer.sin_family      = AF_INET;
  peer.sin_addr.s_addr = servoptions.bindIP;
  sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))
      < 0) return NULL;
    if(bind(sd, (struct sockaddr *)&peer, sizeof(struct sockaddr_in))
      < 0) return NULL;
    if(listen(sd, SOMAXCONN)
      < 0) return NULL;
    for(;;) {
	psz = sizeof(struct sockaddr_in);
        sda = accept(sd, (struct sockaddr *)&peer, &psz);
        if(sda <= 0) continue;
	if(!do_db_check()) {
		sendError(sda,"Database Error: try again later");
		close(sda);
		continue; //TODO: send database error message
	}
	handleConnection(sda);
    }
}
bool do_db_check() {
	int mysql_status = mysql_ping(conn);
	switch(mysql_status) {
		case CR_COMMANDS_OUT_OF_SYNC: {
			servoptions.logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server out of sync\n");
			break;
		}
		case CR_SERVER_GONE_ERROR: {
			servoptions.logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server has gone away\n");
			break;
		}
		case CR_UNKNOWN_ERROR: {
			servoptions.logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server has an unknown connection error\n");
			break;
		}
	}
	mysql_status = mysql_ping(conn); //check if reconnect was successful
	if(mysql_status != 0) {
		servoptions.logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"MySQL server reconnect was unsuccessful\n");
	}
	return mysql_status == 0;
}
bool openspy_mod_query(char *sendmodule, void *data,int len) {
	return false;
}
modInfo *openspy_modInfo() {
	return &moduleInfo;
}
