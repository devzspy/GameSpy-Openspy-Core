#include "main.h"
#include "Client.h"
MYSQL *conn;
modInfo moduleInfo = {"legacyms","GameSpy Legacy Master Server"};
modLoadOptions servoptions;
serverInfo server;
int getnfds(fd_set *rset) {
	int hsock = 0;
	Client *c;
	std::list<Client *>::iterator iterator=server.client_list.begin();
	while(iterator != server.client_list.end()) {
		c=*iterator;
		int sock = c->getSocket();
		if(sock > hsock) hsock = sock;
		FD_SET(sock,rset);
		iterator++;
	}
	return hsock;
}
void checkPing(Client *c) {
}
void processClients(fd_set *rset) {
	Client *c;
	std::list <Client *> clist = server.client_list;
	std::list<Client *>::iterator iterator=clist.begin();
	while(iterator != clist.end()) {
		c=*iterator;
		checkPing(c);
		c->processConnection(rset);
		iterator++;
	}
}
void *openspy_mod_run(modLoadOptions *options)
{
  int sda,sd;
  socklen_t psz;
  fd_set  rset;
  struct sockaddr_in peer;
  struct sockaddr_in user;
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
  peer.sin_port        = htons(LEGACYMSPORT);
  peer.sin_family      = AF_INET;
  peer.sin_addr.s_addr = servoptions.bindIP;
	
  sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))
      < 0) return NULL;
    if(bind(sd, (struct sockaddr *)&peer, sizeof(struct sockaddr_in))
      < 0) return NULL;
    if(listen(sd, SOMAXCONN)
      < 0) return NULL;
    struct timeval timeout;
    memset(&timeout,0,sizeof(struct timeval));
    for(;;) {
	int hsock;
	FD_ZERO(&rset);
	FD_SET(sd, &rset);
	hsock = getnfds(&rset);
	if(hsock < sd) hsock = sd;
	timeout.tv_sec = SB_TIMEOUT_TIME;
	if(select(hsock+1, &rset, NULL, NULL, &timeout)
          < 0) continue;
        if(FD_ISSET(sd, &rset)) {
	} else {
		processClients(&rset);
		continue;
	}
	psz = sizeof(struct sockaddr_in);
        sda = accept(sd, (struct sockaddr *)&user, &psz);
        if(sda <= 0) continue;
	if(!do_db_check()) {
		close(sda);
		continue; //TODO: send database error message
	}
	Client *c = new Client(sda,(struct sockaddr_in *)&user);
	server.client_list.push_front(c);
    }
}
bool openspy_mod_query(char *sendmodule, void *data,int len) {
	return false;
}
modInfo *openspy_modInfo() {
	return &moduleInfo;
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
