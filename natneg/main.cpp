#include "main.h"
#include "server.h"
#include "Client.h"
modInfo moduleInfo = {"natneg","Gamespy NAT Negotiation clone"};
modLoadOptions servoptions;
serverInfo server;
void handleConnection(int sd, struct sockaddr_in *peer, int instance, char *buff, int len) {
	char send[256];
	char *p = (char *)&send;
	int blen = 0;
	int slen = sizeof(sockaddr_in);
	Client *user = find_user(peer,0);
	if(user == NULL) { //unregistered user, create
		user = new Client(sd,peer,instance);
		server.client_list.push_back(user);
	}
	user->handleIncoming(buff,len);
}
void checkTimeouts() {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	while(iterator != server.client_list.end()) {
		user=*iterator;
		if(time(NULL)-NN_PING_TIME > user->getLastPacket()) {
			deleteClient(user);
			iterator = server.client_list.begin();
			continue;
		}
		iterator++;
	}
	iterator=server.client_list.begin();
	while(iterator != server.client_list.end()) {
		user = *iterator;
		if(user->getConnected()) {
			if(!user->getConnectedAck()) {
				if(time(NULL)-5 > user->getSendConnectTime()) {
					user->trySendConnect(false);
				}
			}
		}
		iterator++;
	}
	iterator=server.client_list.begin();
	while(iterator != server.client_list.end()) {
		user=*iterator;
		if(time(NULL)-NN_DEADBEAT_TIME > user->getConnectTime()) {
			if(!user->getConnected()) {
				user->sendDeadBeatNotice();
			}
		}
		iterator++;
	}
}
int getnfds(fd_set *rset, int *sockets, int num_instances) {
	int rs = 0;
	FD_ZERO(rset);
	for(int i=0;i<num_instances;i++) {
		if(sockets[i] > rs) rs = sockets[i];
		FD_SET(sockets[i], rset);
	}
	return rs;	
}
int getIP(int instance) {
	char bindstr[256];
	sprintf_s(bindstr,sizeof(bindstr),"bindip%d",instance+1);
	return servoptions.getConfInt(servoptions.moduleArray,bindstr);
	
}
int getProbeIP() {
	return servoptions.getConfInt(servoptions.moduleArray,"probeip");	
}
void *openspy_mod_run(modLoadOptions *options)
{
#ifdef _WIN32
	WSADATA wsdata;
	WSAStartup(MAKEWORD(2,2),&wsdata);
#endif
  fd_set  rset;
  memcpy(&servoptions,options,sizeof(modLoadOptions));
  int num_instances = options->getConfInt(options->moduleArray,"numinstances");
  char buff[1024];
  int len;
  struct timeval timeout;
  memset(&timeout,0,sizeof(struct timeval));
  if(num_instances < 1) return NULL;
  int *sockets = (int *)malloc(num_instances * sizeof(int));
  struct sockaddr_in *si_me = (struct sockaddr_in *)malloc(num_instances * sizeof(struct sockaddr_in));
  for(int i=0;i<num_instances;i++) {
	if((sockets[i] = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		servoptions.logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"Error creating socket for instance %d\n",i+1);
		return NULL;
	}
	si_me[i].sin_family = AF_INET;
	si_me[i].sin_port = htons(MATCHUP_PORT);
	si_me[i].sin_addr.s_addr = getIP(i);
	if(bind(sockets[i],(struct sockaddr *)&si_me[i],sizeof(struct sockaddr)) == -1) {
		servoptions.logMessageProc(moduleInfo.name,LOGLEVEL_ERROR,"Error binding address for socket instance %d\n",i+1);
		return NULL;
	}
  }
  for(;;) {
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(struct sockaddr_in);
	int rsock = getnfds(&rset,sockets,num_instances);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	if(select(rsock+1, &rset, NULL, NULL, &timeout) < 0) continue;
	for(int i=0;i<num_instances;i++) {
		if(FD_ISSET(sockets[i],&rset)) {
			len = recvfrom(sockets[i],(char *)&buff,sizeof(buff),0,(struct sockaddr *)&si_other,&slen);
			handleConnection(sockets[i], (struct sockaddr_in *)&si_other, i+1, (char *)&buff, len);	
		}
	}
	checkTimeouts();
  }
  return NULL;
}
modInfo *openspy_modInfo() {
	return &moduleInfo;
}
bool openspy_mod_query(char *sendmodule, void *data,int len) {
	return false;
}
