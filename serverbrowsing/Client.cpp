#include "Client.h"
#include "server.h"
Client::Client(int sd, struct  sockaddr_in peer) {
	this->sd = sd;
	querygame = NULL;
	gamename = NULL;
	filter = NULL;
	fieldlist = NULL;
	memset(&challenge,0,sizeof(challenge));
	srcip = 0;
	maxservers = 0;
	fromgamever = 0;
	listversion = 0;
	encodingversion = 0;
	options = 0;
	fromip = 0;
	memcpy(&this->peer,&peer,sizeof(sockaddr_in));
	memset(&encxkeyb,0,sizeof(encxkeyb));
	lastKeepAlive = 0;
	game = NULL;
	queryGame = NULL;
	cryptHeaderSent = false;
	headerLen = 0;
	nextMsgMsgReq = false;
}
void Client::processConnection(fd_set *rset) {
	char buf[MAX_OUTGOING_REQUEST_SIZE + 1];
	char type[128];
	int len;
	if(!FD_ISSET(sd,rset)) {
		return;
	}
	len = recv(sd,buf,sizeof(buf),MSG_NOSIGNAL);
//	makeStringSafe((char *)&buf, sizeof(buf));
	/*
	if(len == -1 && time(NULL)-120 > lastKeepAlive) {
		deleteClient(this);
		return;
	} else if(len == -1) { //timeout, send keep-alive
		char *p = (char *)&buf;
		BufferWriteShortRE((uint8_t**)&p,(uint32_t *)&len,3); //message length(2 + 1 for type)
		BufferWriteByte((uint8_t**)&p,(uint32_t *)&len,KEEPALIVE_MESSAGE);
		enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)p,len);
		if(send(sd,buf,len,MSG_NOSIGNAL) < 0) {
			deleteClient(this);
			return;
		}		
	} else*/ if(len==0 || len == -1) { //disconnected
		deleteClient(this);
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
	handleData((uint8_t *)&buf,(uint32_t)len);
}
Client::~Client() {
	if(querygame != NULL) {
		free((void *)querygame);
	}
	if(gamename != NULL) {
		free((void *)gamename);
	}
	if(filter != NULL) {
		free((void *)filter);
	}
	if(fieldlist != NULL) {
		free((void *)fieldlist);
	}
	close(sd);
}
void Client::handleData(uint8_t *buff, uint32_t len) {
	int remaining = len;
	this->lastKeepAlive = time(NULL);
	while(remaining) {
//		printf("found packet type: %02X %d %d\n",buff[2],remaining,len);
		if(nextMsgMsgReq == true) {
			nextMsgMsgReq = false;
			qrClientMsg cmsg;
			cmsg.data = buff;			
			cmsg.len = remaining;
			qrServerMsg msg;
			msg.msgID = EQRMsgID_ClientMessage;
			msg.data = &cmsg;
			cmsg.srcip = peer.sin_addr.s_addr;
			cmsg.toip = msgIP;
			cmsg.toport = msgPort;
			remaining = 0; //for now just assume that the msg data is everything in the packet
			servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
		} else if(buff[2] == SERVER_LIST_REQUEST) {
			remaining = handleListRequest((uint8_t *)buff,(uint32_t)remaining);
		} else if(buff[2] == SERVER_INFO_REQUEST) {
			remaining = handleInfoRequest((uint8_t *)buff,(uint32_t)remaining);
		} else if(buff[2] == KEEPALIVE_REPLY) {
			remaining = 0;			
		} else if(buff[2] == SEND_MESSAGE_REQUEST) {
			remaining = handleMessageRequest((uint8_t *)buff,(uint32_t)remaining);
		} else { //unknown packet
			printf("found unknown packet %02X(%d) packet len: %d, %d remaining, exiting\n",buff[2],buff[2],len,remaining);
			break;
		}
		buff += (len-remaining);
	}
}
int Client::handleMessageRequest(uint8_t *buff, uint32_t len) {
	buff += 3; //skip the length and request type
	len -= 3;
	msgIP = BufferReadInt(&buff,&len);
	msgPort = BufferReadShort(&buff,&len);
	nextMsgMsgReq = true;
/*
	char sbuff[256];
	uint16_t slen = recv(sd,&sbuff,sizeof(sbuff),0);
	printf("%d is what recv returned\n",slen);
	if(slen < 1) return len;
	qrClientMsg cmsg;
	cmsg.data = (void *)&sbuff;
	cmsg.len = slen;
	qrServerMsg msg;
	msg.msgID = EQRMsgID_ClientMessage;
	msg.data = &cmsg;
	cmsg.srcip = peer.sin_addr.s_addr;
	cmsg.toip = ip;
	cmsg.toport = port;
*/
	//TODO: make this so that it knows a msg is coming and send it upon recieving the next packet to prevent lag, etc
	//servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
	return len;
}
int Client::handleListRequest(uint8_t *buff, uint32_t len) {
	buff += 3; //skip the length and request type
	len -= 3;
	uint8_t listversion,encodingversion;
	uint32_t fromgamever; //game version, just ignore really
	uint32_t srcip = 0;
	uint32_t maxServers = 0;
	uint32_t options;
	listversion = BufferReadByte(&buff,&len);
	encodingversion = BufferReadByte(&buff,&len);
	fromgamever = BufferReadInt(&buff,&len);
	if(querygame == NULL)
	querygame = BufferReadNTS(&buff,&len);
	if(!querygame) goto end;
	if(gamename == NULL)
	gamename = BufferReadNTS(&buff,&len); //game which we are using for encryption
	if(!gamename) goto end;
	BufferReadData(&buff,&len,(uint8_t *)&this->challenge,LIST_CHALLENGE_LEN);
	if(filter == NULL)
	filter = BufferReadNTS(&buff,&len);
	if(!filter) goto end;
	if(fieldlist == NULL)
	fieldlist = BufferReadNTS(&buff,&len);
	if(!fieldlist) goto end;
	options = BufferReadIntRE(&buff,&len);
	if(options & ALTERNATE_SOURCE_IP) {
		srcip = BufferReadInt(&buff,&len);
	}
	if(options & LIMIT_RESULT_COUNT) {
		maxServers = BufferReadInt(&buff,&len);
	}
	this->srcip = srcip;
	this->maxservers = maxServers;
	fromip = peer.sin_addr.s_addr;
	memcpy(&challenge,challenge,sizeof(challenge));
	game = servoptions.gameInfoNameProc((char *)gamename);
	queryGame = servoptions.gameInfoNameProc((char *)querygame);
	char errmsg[128];
	if(game != NULL && game->servicesdisabled != 0) {
		uint8_t *buff,*p;
		uint32_t len = 0;
		buff = (uint8_t *)malloc(128);
		if(buff == NULL) return 0;
		p = buff;
		switch(game->servicesdisabled) {
			case 1: {
				sprintf(errmsg, "Query Error: Services for this game are disabled.");
				break;			
			}
			default:
			case 2: {
				sprintf(errmsg, "Query Error: Services for this game are temporarily disabled.");
				break;
			}
		}
		BufferWriteNTS(&buff, &len, (uint8_t *)&errmsg);
		send(sd,(const char *)p,len,MSG_NOSIGNAL);
		free((void *)p);
		deleteClient(this);
		return len;
	} else if(queryGame != NULL && queryGame->servicesdisabled != 0) {
		uint8_t *buff,*p;
		uint32_t len = 0;
		buff = (uint8_t *)malloc(128);
		if(buff == NULL) return 0;
		p = buff;
		switch(queryGame->servicesdisabled) {
			case 1: {
				sprintf(errmsg, "Query Error: Services for the target game are disabled.");
				break;			
			}
			default:
			case 2: {
				sprintf(errmsg, "Query Error: Services for the target game are temporarily disabled.");
				break;
			}
		}
		BufferWriteNTS(&buff, &len, (uint8_t *)&errmsg);
		send(sd,(const char *)p,len,MSG_NOSIGNAL);
		free((void *)p);
		deleteClient(this);
		return len;
	}
	this->options = options;
	if(this->options & SEND_GROUPS) {
		sendGroups();
	} else if(this->options & NO_SERVER_LIST) {
		sendIP();
	} else {
		sendServers();
		if(this->options & PUSH_UPDATES) {
			sendKeyList();
		}
	} 
	end:
	return len;
}
void Client::sendIP() {
	uint8_t *buff,*p;
	uint32_t len = 0;
	buff = (uint8_t *)malloc(128);
	if(buff == NULL) return;
	p = buff;
	if(game == NULL || queryGame == NULL) {
		BufferWriteNTS(&buff, &len, (uint8_t *)"Query Error: Invalid gamename or clientname");
		send(sd,(const char *)buff,len,MSG_NOSIGNAL);
		free((void *)p);
		deleteClient(this);
		return;
	}
	if(!cryptHeaderSent) {
		setupCryptHeader(&buff, &len);
	}
	BufferWriteInt(&buff,&len,fromip);
	BufferWriteShort(&buff,&len,queryGame->queryport); //default query port, for group listing its always 0
	cryptHeaderSent = true;
	enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)p+headerLen,len-headerLen);
	headerLen = 0;
	send(sd,(const char *)p,len,MSG_NOSIGNAL);
	free((void *)p);
	return;
}
void Client::sendGroups() {
	uint8_t *buff,*p;
	uint16_t num_params = 0;
	uint32_t len = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[256];
	char field[MAX_FIELD_LIST_LEN + 1];
	int fi = 0;
	bool sendPacket = false;
	buff = (uint8_t *)malloc(MAX_OUTGOING_REQUEST_SIZE * 2); //* 2 just in case it goes over the buffer size
	if(buff == NULL) return;
	p = buff;
	if(!do_db_check()) {
		BufferWriteNTS(&buff, &len, (uint8_t *)"Database Error: Database Connection Lost");
		send(sd,(const char *)p,len,MSG_NOSIGNAL);
		free((void *)p);
		deleteClient(this);
		return;
	}
	if(game == NULL || queryGame == NULL) {
		BufferWriteNTS(&buff, &len, (uint8_t *)"Query Error: Invalid gamename or clientname");
		send(sd,(const char *)p,len,MSG_NOSIGNAL);
		free((void *)p);
		deleteClient(this);
		return;
	}
	if(!cryptHeaderSent) {
		setupCryptHeader(&buff, &len);
	}
	BufferWriteInt(&buff,&len,fromip);
	BufferWriteShort(&buff,&len,0); //default query port, for group listing its always 0
	num_params = countchar((char *)fieldlist,'\\');
	BufferWriteShort(&buff,&len,num_params);
	for(int i=0;i<num_params;i++) {
		if(find_param(i, (char *)fieldlist, (char *)&field, sizeof(field))) {
			BufferWriteNTS(&buff, &len, (uint8_t *)&field);
			BufferWriteByte(&buff,&len,0); //on gamespys server there is a bug where a 2nd null byte is put after each word here so do it too for compatibility
		}
	}
	sprintf_s(query,sizeof(query),"SELECT `groupid`,`name`,`maxwaiting`,`other` FROM `Gamemaster`.`grouplist` WHERE `gameid` = '%d'",queryGame->id);
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		free((void *)p);
		deleteClient(this);
		return;
	}
	bool hasMaxServers = maxservers != 0;
	res = mysql_store_result(conn);
	while((row = mysql_fetch_row(res)) != NULL && (!hasMaxServers || maxservers--)) {
		addGroupBuff((char **)&buff,(int *)&len ,(char *)fieldlist,row);
		if(len > MAX_OUTGOING_REQUEST_SIZE) {
			cryptHeaderSent = true;
			enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)p+headerLen,len-headerLen);
			send(sd,(const char *)p,len,MSG_NOSIGNAL);
			headerLen = 0;
			len = 0;
			buff = p;
		}
	}
	mysql_free_result(res);
	BufferWriteByte((uint8_t **)&buff,(uint32_t *)&len,0x00);
	BufferWriteInt(&buff,&len,-1);
	cryptHeaderSent = true;
	enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)p+headerLen,len-headerLen);
	headerLen = 0;
	send(sd,(const char *)p,len,MSG_NOSIGNAL);
	free((void *)p);
}
void Client::addGroupBuff(char **buff,int *len, char *fieldList, MYSQL_ROW row) {
	peerchatMsgData peerchatMsg;
	msgNumUsersOnChan *numUsersMsg;
	char field[MAX_FIELD_LIST_LEN + 1],fielddata[MAX_FIELD_LIST_LEN + 1];
	int i=0;
	BufferWriteByte((uint8_t **)buff,(uint32_t *)len,HAS_KEYS_FLAG);
	BufferWriteIntRE((uint8_t **)buff,(uint32_t *)len,atoi(row[0]));
	while(find_param(i++, (char *)fieldList, (char *)&field, sizeof(field))) {
		BufferWriteByte((uint8_t **)buff,(uint32_t *)len,0xFF);
		if(strcasecmp(field,"hostname") == 0) {
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)row[1]);
		} else if(strcasecmp(field,"numplayers") == 0) { //number of peerchat users in channel
			peerchatMsg.msgid = (char)EMsgID_NumUsersOnChan;
			numUsersMsg = (msgNumUsersOnChan *)malloc(sizeof(msgNumUsersOnChan));
			if(numUsersMsg == NULL) continue;
			peerchatMsg.data = (void *)numUsersMsg;
			memset(numUsersMsg,0,sizeof(msgNumUsersOnChan));
			sprintf_s(numUsersMsg->channelName,sizeof(numUsersMsg->channelName),"#GPG!%u",atoi(row[0]));
			numUsersMsg->showInvisible = false;
			servoptions.sendMsgProc(moduleInfo.name,"peerchat",(void *)&peerchatMsg,sizeof(peerchatMsgData));
			sprintf_s(numUsersMsg->channelName,sizeof(numUsersMsg->channelName),"%d",numUsersMsg->numusers);
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)numUsersMsg->channelName);
			free(numUsersMsg);
		} else if(strcasecmp(field,"numservers") == 0) {
			int numservers = getNumberOfServers(atoi(row[0]));
			char stext[32];	
			sprintf_s(stext,sizeof(stext),"%d",numservers);
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)stext);
		}  else if(strcasecmp(field,"numwaiting") == 0) {
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)"0");
		} else if(strcasecmp(field,"maxwaiting") == 0) {
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)row[2]);
		} else {
			if(row[3] != NULL) {
				if(find_param(field,row[3],(char *)&fielddata,sizeof(fielddata))) {
					BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)&fielddata);
				} else {
					BufferWriteByte((uint8_t **)buff,(uint32_t *)len,0x00);
				}
			}
		}
	}
	return;
}
char *Client::findServerValue(char *name,serverList list) {
	std::list<customKey *>::iterator skeys = list.serverKeys.begin();
	customKey *key;
	if(name == NULL) return NULL;
	while(skeys != list.serverKeys.end()) {
		key = *skeys;
		if(key != NULL) {
			if(key->name != NULL) {
				if(strcmp(key->name,name) == 0) {
					return key->value;
				}
			}
		}
		skeys++;
	}
	return NULL;
}
void Client::addServerBuff(char **buff,int *len, serverList slist) {
	peerchatMsgData peerchatMsg;
	msgNumUsersOnChan *numUsersMsg;
	char field[MAX_FIELD_LIST_LEN + 1],fielddata[MAX_FIELD_LIST_LEN + 1];
	char *fdata;
	int i=0;
	uint8_t flags = 0;
	if((fdata = findServerValue("natneg",slist)) != NULL) {
		if(atoi(fdata) != 0) {
			flags |= CONNECT_NEGOTIATE_FLAG; //not really used in the sdk it seems though
		}
	}
	uint32_t privip = 0;
	uint16_t hostport = slist.port;
	uint16_t localport = 0;
	if((fdata = findServerValue("localip0",slist)) != NULL) {
		int addr = inet_addr(fdata);
		if(addr != slist.ipaddr) {
			flags |= PRIVATE_IP_FLAG;
			privip = addr;
		}
	}
	if((fdata = findServerValue("hostport",slist)) != NULL) {
		hostport = htons(atoi(fdata));
	}
	if((fdata = findServerValue("localport",slist)) != NULL) {
		localport = htons(atoi(fdata));
		if(localport != htons(queryGame->queryport)) {
			flags |= NONSTANDARD_PRIVATE_PORT_FLAG;
		}
	}
	if(hostport != htons(queryGame->queryport)) {
		flags |= NONSTANDARD_PORT_FLAG;
	}
	/*
	if((fdata = findServerValue("firewall",slist)) != NULL) {
		if(atoi(fdata) != 0)
			flags |= UNSOLICITED_UDP_FLAG;
	}
	*/
	if(fieldlist != NULL) flags |= HAS_KEYS_FLAG;
	BufferWriteByte((uint8_t **)buff,(uint32_t *)len,flags);
	BufferWriteInt((uint8_t **)buff,(uint32_t *)len,slist.ipaddr);
	if(flags & NONSTANDARD_PORT_FLAG) {
		BufferWriteShort((uint8_t **)buff,(uint32_t *)len,hostport);
	}
	if(flags & PRIVATE_IP_FLAG) {
		BufferWriteInt((uint8_t **)buff,(uint32_t *)len,privip);
	}
	if(flags & NONSTANDARD_PRIVATE_PORT_FLAG) {
		BufferWriteShort((uint8_t **)buff,(uint32_t *)len,localport);
	}
	while(find_param(i++, (char *)fieldlist, (char *)&field, sizeof(field))) {
		BufferWriteByte((uint8_t **)buff,(uint32_t *)len,0xFF);
		if(strcasecmp(field,"country") == 0) {
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)slist.country->countrycode);				
		} else if(strcasecmp(field,"region") == 0) {
			sprintf(fielddata,"%d",slist.country->region);
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)fielddata);				
		} else if((fdata = findServerValue((char *)&field,slist)) != NULL) {
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)fdata);	
		} else {
			BufferWriteByte((uint8_t **)buff,(uint32_t *)len,0x00);
		}
	}
}
void Client::sendServers() {
	uint8_t buff[MAX_OUTGOING_REQUEST_SIZE * 2];
	uint8_t *p;
	uint16_t num_params = 0;
	uint32_t len = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char field[MAX_FIELD_LIST_LEN + 1];
	int fi = 0;
	bool sendPacket = false;
	p = (uint8_t *)&buff;
	if(game == NULL || queryGame == NULL) {
		BufferWriteNTS(&p, &len, (uint8_t *)"Query Error: Invalid gamename or clientname");
		send(sd,(const char *)buff,len,MSG_NOSIGNAL);
		deleteClient(this);
		return;
	}
	if(!cryptHeaderSent) {
		setupCryptHeader(&p, &len);
	}
	BufferWriteInt(&p,&len,fromip);
	BufferWriteShortRE(&p,&len,queryGame->queryport); //default query port, for group listing its always 0
	num_params = countchar((char *)fieldlist,'\\');
	BufferWriteShort(&p,&len,num_params);
	for(int i=0;i<num_params;i++) {
		if(find_param(i, (char *)fieldlist, (char *)&field, sizeof(field))) {
			BufferWriteNTS(&p, &len, (uint8_t *)&field);
			BufferWriteByte(&p,&len,0); //on gamespys server there is a bug where a 2nd null byte is put after each word here so do it too for compatibility
		}
	}
	qrServerMsg msg;
	qrServerList listData;
	msg.data = (void *)&listData;
	listData.game = queryGame;
	listData.filter = filter;
	msg.msgID = EQRMsgID_GetServer;
	servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
	std::list<serverList>::iterator iterator = listData.server_list.begin();
	serverList slist;
	bool hasMaxServers = maxservers != 0;
	int maxservers = this->maxservers;
	while(iterator != listData.server_list.end() && (!hasMaxServers || maxservers--) && listData.numServers--) {
		slist = *iterator;
		addServerBuff((char**)&p,(int *)&len,slist);
		if(len > MAX_OUTGOING_REQUEST_SIZE) {
			cryptHeaderSent = true;
			enctypex_func6e((unsigned char *)&encxkeyb,((unsigned char *)&buff)+headerLen,len-headerLen);
			send(sd,(const char *)buff,len,MSG_NOSIGNAL);
			headerLen = 0;
			len = 0;
			p = (uint8_t *)&buff;
		}
		iterator++;
	}
	BufferWriteByte((uint8_t **)&p,(uint32_t *)&len,0x00);
	BufferWriteInt((uint8_t **)&p,(uint32_t *)&len,-1);
	cryptHeaderSent = true;
	enctypex_func6e((unsigned char *)&encxkeyb,((unsigned char *)&buff)+headerLen,len-headerLen);
	headerLen = 0;
	send(sd,(const char *)buff,len,MSG_NOSIGNAL);
	freeServerRuleList(slist.serverKeys);
	return;
}
void Client::setupCryptHeader(uint8_t **dst, uint32_t *len) { 
//	memset(&options->cryptkey,0,sizeof(options->cryptkey));
	srand(time(NULL));
	uint32_t cryptlen = CRYPTCHAL_LEN;
	uint8_t cryptchal[CRYPTCHAL_LEN];
	uint32_t servchallen = SERVCHAL_LEN;
	uint8_t servchal[SERVCHAL_LEN];
	headerLen = (servchallen+cryptlen)+(sizeof(uint8_t)*2);
	uint16_t *backendflags = (uint16_t *)(&cryptchal);
	for(uint32_t i=0;i<cryptlen;i++) {
		cryptchal[i] = (uint8_t)rand();		
	}
	*backendflags = queryGame->backendflags;
	for(uint32_t i=0;i<servchallen;i++) {
		servchal[i] = (uint8_t)rand();		
	}
	BufferWriteByte(dst,len,cryptlen^0xEC);
	BufferWriteData(dst, len, (uint8_t *)&cryptchal, cryptlen);
	BufferWriteByte(dst,len,servchallen^0xEA);
	BufferWriteData(dst, len, (uint8_t *)&servchal, servchallen);
	enctypex_funcx((unsigned char *)&encxkeyb, (unsigned char *)game->secretkey,(unsigned char *)challenge, (unsigned char *)&servchal,servchallen);
}
int Client::handleInfoRequest(uint8_t *buff, uint32_t len) {
	uint32_t ip;
	uint16_t port;
	qrServerRules qrRules;
	qrServerMsg msg;
	buff += 3; //skip the length and request type
	len -= 3;
	ip = BufferReadInt(&buff,&len);
	port = BufferReadShort(&buff,&len);
	qrRules.game = queryGame;
	msg.msgID = EQRMsgID_GetServerRules;
	msg.data = (void *)&qrRules;
	qrRules.ipaddr = (ip);
	qrRules.port = (port);
	servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
	sendServerRules(qrRules.server_rules,ip,port);
	return len;
}
void Client::sendServerRules(std::list<customKey *> server_rules,uint32_t ip, uint16_t port) {
	std::list<customKey *>::iterator it;
	uint8_t outbuff[1024],*p,*x;
	uint32_t len = 0;
	uint8_t flags = 0;
	serverList slist;
	customKey *key;
	p = (uint8_t *)&outbuff;
	x = (uint8_t *)&outbuff;
	char *fdata;
	if(game == NULL || queryGame == NULL) {
		BufferWriteNTS(&p, &len, (uint8_t *)"Query Error: Invalid gamename or clientname");
		send(sd,(const char *)outbuff,len,MSG_NOSIGNAL);
		free((void *)p);
		deleteClient(this);
		return;
	}
	if(!cryptHeaderSent) {
		setupCryptHeader(&p, &len);
	}
	slist.serverKeys = server_rules;
	if((fdata = findServerValue("natneg",slist)) != NULL) {
		if(atoi(fdata) != 0) {
			flags |= CONNECT_NEGOTIATE_FLAG; //not really used in the sdk it seems
		}
	}
	uint32_t privip = 0;
	uint16_t hostport = port;
	uint16_t localport = 0;
	if((fdata = findServerValue("localip0",slist)) != NULL) {
		int addr = inet_addr(fdata);
		if(addr != slist.ipaddr) {
			flags |= PRIVATE_IP_FLAG;
			privip = addr;
		}
	}
	if((fdata = findServerValue("hostport",slist)) != NULL) {
		hostport = htons(atoi(fdata));
	}
	if((fdata = findServerValue("localport",slist)) != NULL) {
		localport = htons(atoi(fdata));
		if(localport != htons(queryGame->queryport)) {
			flags |= NONSTANDARD_PRIVATE_PORT_FLAG;
		}
	}
	if(hostport != htons(queryGame->queryport)) {
		flags |= NONSTANDARD_PORT_FLAG;
	}
	if((fdata = findServerValue("firewall",slist)) != NULL) {
		if(atoi(fdata) == 0 || ~flags & CONNECT_NEGOTIATE_FLAG)
			flags |= UNSOLICITED_UDP_FLAG;
	}
	flags |= ICMP_IP_FLAG;
	flags |= HAS_FULL_RULES_FLAG;
	//BufferWriteInt(&p,&len,options->fromip);
//	BufferWriteShortRE(&p,&len,options->queryGame->queryport); //default query port, for group listing its always 0
	BufferWriteShortRE(&p,&len,0); //write nothing for now and make note of the length later length - ip+port(6)
	BufferWriteByte(&p,&len,PUSH_SERVER_MESSAGE);
	BufferWriteByte(&p,&len,flags);
	BufferWriteInt(&p,&len,ip);
	if(flags & NONSTANDARD_PORT_FLAG) {
		BufferWriteShort(&p,&len,port);
	}
	if(flags & PRIVATE_IP_FLAG) {
		BufferWriteInt(&p,&len,privip);
	}
	if(flags & NONSTANDARD_PRIVATE_PORT_FLAG) {
		BufferWriteShort(&p,&len,localport);
	}
	BufferWriteInt(&p,&len,ip);	
	it = server_rules.begin();
	while(it != server_rules.end()) {
		key = *it;
		BufferWriteNTS(&p,&len,(uint8_t*)key->name);
		BufferWriteNTS(&p,&len,(uint8_t*)key->value);
		it++;
	}
	freeServerRuleList(server_rules);
	BufferWriteByte(&p,&len,0);
	uint16_t *y = (uint16_t *)x;
	*y = reverse_endian16((uint16_t)len);
	enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)x+headerLen,len-headerLen);
	cryptHeaderSent = true;
	headerLen = 0;
	send(sd,(const char *)x,len,MSG_NOSIGNAL|MSG_DONTWAIT);
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
int Client::getSocket() {
	return sd;
}
struct sockaddr_in *Client::getSockAddr() {
	return (struct sockaddr_in *)&peer;
}
uint32_t Client::getAddress() {
	return peer.sin_addr.s_addr;
}
time_t Client::getLastPing() {
	return lastKeepAlive;
}
void Client::sendKeyList() {
	uint8_t buff[512];
	uint32_t len = 0;
	uint8_t num_keys = queryGame->numPushKeys;
	uint8_t *p = (uint8_t *)&buff;
	if(!cryptHeaderSent) {
		setupCryptHeader(&p, &len);
	}
	BufferWriteShortRE(&p,&len,0); //write nothing(length, will do later when calculated)
	BufferWriteByte(&p,&len,PUSH_KEYS_MESSAGE);
	BufferWriteByte(&p,&len,num_keys);
	for(int i=0;i<num_keys;i++) {
		BufferWriteByte(&p,&len,(uint8_t)queryGame->pushKeys[i].type);
		BufferWriteNTS(&p,&len,(uint8_t*)queryGame->pushKeys[i].name);
	}
	uint16_t *msglen = (uint16_t *)((&buff)+headerLen);
	*msglen = htons(len-headerLen);
	enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)(&buff)+headerLen,len-headerLen);
	cryptHeaderSent = true;
	headerLen = 0;
	send(sd,(char *)&buff,len,MSG_NOSIGNAL|MSG_DONTWAIT);
}
void Client::delServer(serverList slist) {
	uint8_t data[256];
	uint8_t *p = (uint8_t *)(&data);
	uint16_t *olen = (uint16_t *)&data[0];
	uint32_t len = 0;
	if(!cryptHeaderSent) {
		setupCryptHeader(&p, &len);
	}
	BufferWriteShortRE(&p,&len,0); //write nothing(length, will do later when calculated)
	BufferWriteByte(&p,&len,DELETE_SERVER_MESSAGE);
	BufferWriteInt(&p,&len,slist.ipaddr);
	BufferWriteShort(&p,&len,slist.port);
	*olen = htons(len);
	enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)(&data)+headerLen,len-headerLen);
	cryptHeaderSent = true;
	headerLen = 0;
	send(sd,(char *)&data,len,MSG_NOSIGNAL|MSG_DONTWAIT);
}
void Client::pushServer(serverList slist) {
	uint8_t data[256];
	uint32_t len = 0;
	uint8_t flags = 0;
	char *fdata;
	int keysize = queryGame->numPushKeys;
	uint8_t *p = (uint8_t *)&data;
	uint16_t *olen = (uint16_t *)&data[0];
	if(!cryptHeaderSent) {
		setupCryptHeader(&p, &len);
	}
	BufferWriteShortRE(&p,&len,0); //write nothing(length, will do later when calculated)
	BufferWriteByte(&p,&len,PUSH_SERVER_MESSAGE);
	if((fdata = findServerValue("natneg",slist)) != NULL) {
		if(atoi(fdata) != 0) {
			flags |= CONNECT_NEGOTIATE_FLAG; //not really used in the sdk it seems
		}
	}
	uint32_t privip = 0;
	uint16_t hostport = slist.port;
	uint16_t localport = 0;
	if((fdata = findServerValue("localip0",slist)) != NULL) {
		int addr = inet_addr(fdata);
		if(addr != slist.ipaddr) {
			flags |= PRIVATE_IP_FLAG;
			privip = addr;
		}
	}
	if((fdata = findServerValue("hostport",slist)) != NULL) {
		hostport = htons(atoi(fdata));
	}
	if((fdata = findServerValue("localport",slist)) != NULL) {
		localport = htons(atoi(fdata));
		if(localport != htons(queryGame->queryport)) {
			flags |= NONSTANDARD_PRIVATE_PORT_FLAG;
		}
	}
	if(hostport != htons(queryGame->queryport)) {
		flags |= NONSTANDARD_PORT_FLAG;
	}
	if((fdata = findServerValue("firewall",slist)) != NULL) {
		if(atoi(fdata) != 0)
			flags |= UNSOLICITED_UDP_FLAG;
	}
	if(keysize > 0) {
		flags |= HAS_KEYS_FLAG;
	}
	BufferWriteByte(&p,&len,flags);
	BufferWriteInt(&p,&len,slist.ipaddr);
	if(flags & NONSTANDARD_PORT_FLAG) {
		BufferWriteShort(&p,&len,hostport);
	}
	if(flags & PRIVATE_IP_FLAG) {
		BufferWriteInt(&p,&len,privip);
	}
	if(flags & NONSTANDARD_PRIVATE_PORT_FLAG) {
		BufferWriteShort(&p,&len,localport);
	}
	for(int i=0;i<keysize;i++) {
		if(strcmp(queryGame->pushKeys[i].name,"country") == 0) {
			BufferWriteNTS(&p,&len,(uint8_t*)slist.country->countrycode);
		} else if(queryGame->pushKeys[i].type == KEYTYPE_BYTE) {
			uint8_t value = 0;
			if((fdata = findServerValue(queryGame->pushKeys[i].name,slist)) != NULL) {
				value = atoi(fdata);
			}
			BufferWriteByte(&p,&len,(uint8_t)value);
		} else if(queryGame->pushKeys[i].type == KEYTYPE_SHORT) {
			uint16_t value = 0;
			if((fdata = findServerValue(queryGame->pushKeys[i].name,slist)) != NULL) {
				value = atoi(fdata);
			}
			BufferWriteShort(&p,&len,value);
		} else if(queryGame->pushKeys[i].type == KEYTYPE_STRING) {
			if((fdata = findServerValue(queryGame->pushKeys[i].name,slist)) != NULL) {
				BufferWriteNTS(&p,&len,(uint8_t *)fdata);
			} else {
				BufferWriteByte(&p,&len,0);
			}
		}
	}
	BufferWriteByte(&p,&len,0);
	*olen = htons(len);
	enctypex_func6e((unsigned char *)&encxkeyb,(unsigned char *)(&data)+headerLen,len-headerLen);
	cryptHeaderSent = true;
	headerLen = 0;
	send(sd,(char *)&data,len,MSG_NOSIGNAL|MSG_DONTWAIT);
}
int Client::getNumberOfServers(uint16_t groupid) {
	qrServerMsg msg;
	qrServerList listData;
	msg.data = (void *)&listData;
	listData.game = queryGame;
	char filter[256];
	sprintf_s(filter,sizeof(filter),"groupid=%d",groupid);
	listData.filter = (uint8_t *)&filter;
	msg.msgID = EQRMsgID_GetServer;
	servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
	std::list<serverList>::iterator iterator = listData.server_list.begin();
	int i = 0;
	serverList slist;
	while(iterator != listData.server_list.end() && listData.numServers--) {
		slist = *iterator;
		freeServerRuleList(slist.serverKeys);
		i++;
		iterator++;
	}
	return i;
}
bool Client::wantsUpdates() {
	return (options & PUSH_UPDATES);
}
gameInfo *Client::getQueryGame() {
	return queryGame;
}
uint8_t *Client::getFilter() {
	return filter;
}
