#include "Client.h"
#include "server.h"
#include <common/enctype_shared.h>
extern MYSQL *conn;
Client::Client(int sd, struct sockaddr_in *peer) {
	char sbuff[256];
	int slen = 0;
	memset(&challenge,0,sizeof(challenge));
	memset(&sbuff,0,sizeof(sbuff));
	game = NULL;
	this->sd = sd;
	memcpy(&sockinfo,&peer,sizeof(struct sockaddr_in));
	gen_random((char *)&challenge,6);
	enctype = 0;
	validated = false;
	keyptr = NULL;
	sendbuff = NULL;
	sbuffp = NULL;
	sbuffalloclen = 0;
	sendlen = 0;
	slen = sprintf_s(sbuff,sizeof(sbuff),"\\basic\\\\secure\\%s",challenge);
	//for some reason theres a null byte sent at the end so send it here too
	senddata((char *)&sbuff,slen,false,true,true,true);
}
Client::~Client() {
	senddata(NULL, 0, true, false, false, true ); //flush remaining data
	free((void *)sbuffp);
	close(sd);
}
int Client::getSocket() {
	return sd;
}
struct sockaddr_in *Client::getSockAddr() {
	return &sockinfo;
}
uint32_t Client::getAddress() {
	return sockinfo.sin_addr.s_addr;
}
uint16_t Client::getPort() {
	return sockinfo.sin_port;
}
time_t Client::getLastPing() {
	return lastPing;
}
time_t Client::getConnectTime() {
	return connected;
}
void Client::processConnection(fd_set *rset) {
	char buf[MAX_OUTGOING_REQUEST_SIZE + 1];
	char type[128];
	int len;
	if(!FD_ISSET(sd,rset)) {
		return;
	}
	len = recv(sd,buf,sizeof(buf),MSG_NOSIGNAL);
	if(len == 0 || len == -1) { //disconnected
		deleteClient(this);
		return;
	}
	if(!do_db_check()) {
		return;
	}
	char *p = (char *)&buf;	
	char *x;
	while(true) {
		x = p;
		p = strstr(p,"\\final\\");
		if(p == NULL) { break; }
		*p = 0;
		p+=7;
		handleData((uint8_t *)x,(uint32_t)(x - ((char *)&buf)));
	}
	if((x - ((char *)&buf)) > 7) {
		handleData((uint8_t *)x,(uint32_t)(x - ((char *)&buf)));
	}
//	handleData((uint8_t *)&buf,(uint32_t)len);
}
void Client::handleData(uint8_t *buff,uint32_t len) {
	char cmd[64];
	if(!find_param(0, (char *)buff, (char *)&cmd, sizeof(cmd))) {
		return;
	}
	if(strcmp(cmd,"gamename") == 0 && !validated) { //validation, etc
		handleValidation(buff,len);
	} else if(validated) { 
		if(strcmp(cmd,"list") == 0) {
			handleList(buff,len);
		}
	}
}
void Client::sendCryptHeader() {
	char sbuff[256];
	char *buff = (char *)(&sbuff);
	int slen = 0;
	if(enctype == 1) {
/*
		char encxbuff[16];
		for(int x=0;x<sizeof(encxbuff);x++) {
			encxbuff[x] = (uint8_t)rand()%sizeof(enctype1_cryptdata)-1;
		}
		BufferWriteInt((uint8_t **)&buff,(uint32_t *)&slen,0);//length of entire packet
		BufferWriteByte((uint8_t **)&buff,(uint32_t *)&slen,(5^62)+20);
		BufferWriteByte((uint8_t **)&buff,(uint32_t *)&slen,(3^205)+5);
		for(int x=0;x<13;x++) { //13 bytes of nothingness!
			BufferWriteByte((uint8_t **)&buff,(uint32_t *)&slen,rand());
		}	
		for(int x=0;x<sizeof(encxbuff);x++) {
			BufferWriteByte((uint8_t **)&buff,(uint32_t *)&slen,encxbuff[x]);
		}
		func1(NULL, 0, cryptkey_enctype1);
		func4(challenge, sizeof(challenge), cryptkey_enctype1);
*/
	} else if(enctype == 2) {
		char cryptkey[13];
		int secretkeylen = strlen(game->secretkey);
		for(int x=0;x<sizeof(cryptkey);x++) {
			cryptkey[x] = (uint8_t)rand();
		}
		encshare4((unsigned char *)&cryptkey, sizeof(cryptkey),(unsigned int *)&this->cryptkey_enctype2);
		for(int i=0;i< secretkeylen;i++) {
			cryptkey[i] ^= game->secretkey[i];
		}
		BufferWriteByte((uint8_t **)&buff,(uint32_t *)&slen,sizeof(cryptkey)^0xEC);
		BufferWriteData((uint8_t **)&buff, (uint32_t *)&slen, (uint8_t *)&cryptkey, sizeof(cryptkey));
	}
	senddata((char *)&sbuff, slen, false, false, true, true);
	
}
void Client::handleValidation(uint8_t *buff,uint32_t len) {
	char validation[16],realvalidate[16];
	char gamename[64];
	enctype = find_paramint("enctype",(char *)buff);
	if(enctype < 0 || enctype > 2) {
		deleteClient(this);
		return;
	} 
	find_param("gamename",(char *)buff, (char *)&gamename,sizeof(gamename));
	game = servoptions.gameInfoNameProc((char *)gamename);
	find_param("validate",(char *)buff,(char *)&validation,sizeof(validation));
	gsseckey((unsigned char *)&realvalidate, (unsigned char *)&challenge, (unsigned char *)game->secretkey, enctype);
	if(strcmp(realvalidate,validation) == 0 && game != NULL && game->servicesdisabled == 0) {
		validated = true;
	} else {
		deleteClient(this);
		return;
	}
	if(enctype != 0){
		sendCryptHeader();
	}
}
void Client::handleList(uint8_t *buff,uint32_t len) {
	char type[16];
	char gamename[64];
	char filter[256];
	bool hasFilter = true;
	gameInfo *queryGame = NULL;
	if(!find_param("list",(char *)buff,(char *)&type,sizeof(type))) {
		return;
	}
	if(!find_param("where",(char *)buff,(char *)&filter,sizeof(filter))) {
		hasFilter = false;
	}
	if(!find_param("gamename",(char *)buff,(char *)&gamename,sizeof(gamename))) {
		queryGame = game;
	} else {
		queryGame = servoptions.gameInfoNameProc((char *)&gamename);
	}
	if(queryGame == NULL) return;
	if(strcmp("cmp",type) == 0) {
		sendServers(queryGame,hasFilter==true?((char *)&filter):NULL);
	} else if(strcmp("groups", type) == 0) {
		sendGroups(queryGame);
	} else if(strcmp("info2", type) == 0) {
		//TODO
	}
	deleteClient(this);//appearently the server immidently terminates the connection
}
int getPeerchatUsers(int groupid) {
	int retval;
	peerchatMsgData peerchatMsg;
	msgNumUsersOnChan *numUsersMsg;
	peerchatMsg.msgid = (char)EMsgID_NumUsersOnChan;
	numUsersMsg = (msgNumUsersOnChan *)malloc(sizeof(msgNumUsersOnChan));
	if(numUsersMsg == NULL) return 0;
	peerchatMsg.data = (void *)numUsersMsg;
	memset(numUsersMsg,0,sizeof(msgNumUsersOnChan));
	sprintf_s(numUsersMsg->channelName,sizeof(numUsersMsg->channelName),"#GPG!%u",groupid);
	numUsersMsg->showInvisible = false;
	servoptions.sendMsgProc(moduleInfo.name,"peerchat",(void *)&peerchatMsg,sizeof(peerchatMsgData));
	retval = numUsersMsg->numusers;
	free((void *)numUsersMsg);
	return retval;
}
void Client::sendGroups(gameInfo *queryGame) {
	/*\fieldcount\8\groupid\hostname\numplayers\maxwaiting\numwaiting\numservers\password\other\309\Europe\0\50\0\0\0\.maxplayers.0\408\Pros\0\50\0\0\0\.maxplayers.0\254\West Coast 2\0\50\0\0\0\.maxplayers.0\255\West Coast 3\0\50\0\0\0\.maxplayers.0\256\East Coast 1\0\50\0\0\0\.maxplayers.0\257\East Coast 2\0\50\0\0\0\.maxplayers.0\253\West Coast 1\0\50\0\0\0\.maxplayers.0\258\East Coast 3\0\50\0\0\0\.maxplayers.0\407\Newbies\0\50\0\0\0\.maxplayers.0\final\*/
	#define addBuff(x)  strcat(p,"\\"); p++; \
			    strcat(p,x); p+=strlen(x); 
	char *fielddata = "\\fieldcount\\8\\groupid\\hostname\\numplayers\\maxwaiting\\numwaiting\\numservers\\password\\other";
	char sbuff[256];
	char *p = (char *)&sbuff;
	memset(&sbuff,0,sizeof(sbuff));
	int slen = 0;
	char query[256];
	char otherbuff[256];
	MYSQL_ROW row;
	MYSQL_RES *res;
	memset(&otherbuff,0,sizeof(otherbuff));
	sprintf_s(query,sizeof(query),"SELECT `groupid`,`name`,`maxwaiting`,`password`,`other` FROM `Gamemaster`.`grouplist` WHERE `gameid` = '%d'",queryGame->id);
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		deleteClient(this);
		return;
	}
	senddata(fielddata,strlen(fielddata),false,false);
	res = mysql_store_result(conn);
	while((row = mysql_fetch_row(res)) != NULL) {
		addBuff(row[0])
		addBuff(row[1])
		int numusers = getPeerchatUsers(atoi(row[0]));
		sprintf_s(otherbuff,sizeof(otherbuff),"%u",numusers);
		addBuff(otherbuff)
		addBuff(row[2])
		addBuff("0")
		addBuff("0")
		addBuff(row[3])
		strcpy(otherbuff,row[4]);
		find_and_replace(otherbuff,'\\',0x01);
		addBuff(otherbuff)
		senddata(sbuff,strlen(sbuff),false,false);
		p = (char *)&sbuff;
		memset((char *)&sbuff,0,sizeof(sbuff));
		slen = 0;
	}
	mysql_free_result(res);
	
}
void Client::sendServers(gameInfo *queryGame, char *filter) {
	char sbuff[sizeof(uint32_t) + sizeof(uint16_t)];
	char *buff = (char *)(&sbuff);
	int len = 0; //required for bufferwrite stuff
	qrServerMsg msg;
	qrServerList listData;
	msg.data = (void *)&listData;
	listData.game = queryGame;
	listData.filter = (uint8_t *)filter;
	msg.msgID = EQRMsgID_GetServer;
	servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
	std::list<serverList>::iterator iterator = listData.server_list.begin();
	serverList slist;
	while(iterator != listData.server_list.end()) {
		slist = *iterator;
		BufferWriteInt((uint8_t **)&buff,(uint32_t *)&len,slist.ipaddr);
		BufferWriteShort((uint8_t **)&buff,(uint32_t *)&len,slist.port);
		freeServerRuleList(slist.serverKeys);
		senddata(sbuff,sizeof(sbuff),false,false);
		iterator++;
		buff = (char *)(&sbuff);
	}
	
}
void Client::freeServerRuleList(std::list<customKey *> slist) {
	std::list<customKey *>::iterator it;
	customKey *key;
	it = slist.begin();
	while(it != slist.end()) {
		key = *it;
		if(key->name != NULL) free((void *)key->name);
		if(key->value != NULL) free((void *)key->value);
		free((void *)key);
		it++;
	}
}
void Client::senddata(char *buff, int len, bool sendFinal, bool initPacket, bool noEncryption, bool actualSend ) {
	if(sendbuff == NULL) {
		sendbuff = (char *)malloc(1024);
		sbuffalloclen = 1024;
		sbuffp = sendbuff;
	} else if(len > (sbuffalloclen-128) || ((sendbuff - sbuffp)+len > (sbuffalloclen-128))) {
		char *sptr = sbuffp;
		sbuffp = (char *)realloc(sbuffp,sbuffalloclen+len+1024);
		if(sbuffp != NULL) {
			sbuffalloclen += len+1024;
		} else sbuffp = sptr;
	}
	if(buff != NULL)
		BufferWriteData((uint8_t **)&sendbuff, (uint32_t *)&sendlen, (uint8_t *)buff, len);
	if(sendFinal && enctype != 1) {
		BufferWriteData((uint8_t **)&sendbuff, (uint32_t *)&sendlen, (uint8_t *)"\\final\\", 7);
	}
	if(initPacket) {
		*sendbuff++ = 0;
		sendlen++; //the server sends a nullbyte too for some reason so increase by one
	}
	if(actualSend) {
		if(!noEncryption) {
			switch(enctype) {
				case 1:
					break;
				case 2:
					keyptr = encshare1((unsigned int *)&cryptkey_enctype2, (unsigned char *)sbuffp, sendlen,keyptr);
					break;
				case 0:
				default:
					break;
			}
		}
		send(sd,sbuffp,sendlen,MSG_NOSIGNAL);
		sendbuff = sbuffp;
		sendlen = 0;
	}
}
