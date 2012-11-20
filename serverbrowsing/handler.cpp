#include "handler.h"
void *processConnection(threadOptions *options) {
	handlerOptions handleroptions;
	memset(&handleroptions,0,sizeof(handlerOptions));
	handleroptions.lastKeepAlive = time(NULL);
	bool firstPacket = true;
	char buf[MAX_OUTGOING_REQUEST_SIZE + 1];
	char type[128];
	int len;
	int sd = options->sd;
	struct sockaddr_in peer;
	memcpy(&handleroptions.peer,&options->peer,sizeof(struct sockaddr_in));
	handleroptions.sd = sd;
	free((void *)options);
	struct timeval tv;
	while(!handleroptions.terminated) {
		tv.tv_sec = 60;
		tv.tv_usec = 0;
		setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv);
		len = recv(sd,buf,sizeof(buf),MSG_NOSIGNAL);
//		makeStringSafe((char *)&buf, sizeof(buf));
		if(len == -1 && time(NULL)-120 > handleroptions.lastKeepAlive) {
			handleroptions.terminated = true;
			close(sd);
			break;
		} else if(len == -1) { //timeout, send keep-alive
			char *p = (char *)&buf;
			BufferWriteShortRE((uint8_t**)&p,(uint32_t *)&len,3); //message length(2 + 1 for type)
			BufferWriteByte((uint8_t**)&p,(uint32_t *)&len,KEEPALIVE_MESSAGE);
//			GOAEncrypt(&(handleroptions.cryptkey), (unsigned char *)p, len);
			enctypex_func6e((unsigned char *)&handleroptions.encxkeyb,(unsigned char *)p,len);
			if(send(options->sd,buf,len,MSG_NOSIGNAL) < 0) {
				handleroptions.terminated = true;
				close(sd);
			}
			continue;			
		} else if(len == 0) { //disconnected
			handleroptions.terminated = true;
		}
		firstPacket = false;
//		printf("found packet type: %02X\n",buf[2]);
		switch(buf[2]) {
			case SERVER_LIST_REQUEST:
				handleListRequest((uint8_t *)&buf,(uint32_t)len, &handleroptions);
			case KEEPALIVE_REPLY:
				handleroptions.lastKeepAlive = time(NULL);			
				break;
			case SERVER_INFO_REQUEST:
				handleInfoRequest((uint8_t *)&buf,(uint32_t)len, &handleroptions);
			default:
				return NULL;
		}
	}
	return NULL;
}
void handleListRequest(uint8_t *buff, uint32_t len, handlerOptions *handleroptions) {
	buff += 3; //skip the length and request type
	len -= 3;	
	uint8_t *querygame = NULL,*gamename = NULL,*filter = NULL,*fieldlist = NULL;
	uint8_t listversion,encodingversion;
	uint32_t fromgamever; //game version, just ignore really
	uint8_t challenge[9];
	uint32_t srcip = 0;
	uint32_t maxServers = 0;
	memset(&challenge,0,sizeof(challenge));
	uint32_t options;
	listversion = BufferReadByte(&buff,&len);
	encodingversion = BufferReadByte(&buff,&len);
	fromgamever = BufferReadInt(&buff,&len);
	querygame = BufferReadNTS(&buff,&len);
	if(!querygame) goto end;
	gamename = BufferReadNTS(&buff,&len); //game which we are using for encryption
	if(!gamename) goto end;
	BufferReadData(&buff,&len,(uint8_t *)&challenge,LIST_CHALLENGE_LEN);
	filter = BufferReadNTS(&buff,&len);
	if(!filter) goto end;
	fieldlist = BufferReadNTS(&buff,&len);
	if(!fieldlist) goto end;
	options = BufferReadIntRE(&buff,&len);
	if(options & ALTERNATE_SOURCE_IP) {
		srcip = BufferReadInt(&buff,&len);
	}
	if(options & LIMIT_RESULT_COUNT) {
		maxServers = BufferReadInt(&buff,&len);
	}
	handleroptions->querygame = querygame;
	handleroptions->gamename = gamename;
	handleroptions->filter = filter;
	handleroptions->fieldlist = fieldlist;
	handleroptions->srcip = srcip;
	handleroptions->maxservers = maxServers;
	handleroptions->fromip = handleroptions->peer.sin_addr.s_addr;
	memcpy(&handleroptions->challenge,challenge,sizeof(handleroptions->challenge));
	handleroptions->game = servoptions.gameInfoNameProc((char *)handleroptions->gamename);
	handleroptions->queryGame = servoptions.gameInfoNameProc((char *)handleroptions->querygame);
	handleroptions->options = options;
	if(handleroptions->options & SEND_GROUPS) {
		sendGroups(handleroptions);
	} else {
		sendServers(handleroptions);
	} 
	end:
	if(querygame)
		free((void *)querygame);
	if(gamename)
		free((void *)gamename);
	if(filter)
		free((void *)filter);
	if(fieldlist)
		free((void *)fieldlist);
	return;
}
void sendGroups(handlerOptions *options) {
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
	if(options->game == NULL || options->queryGame == NULL) {
		BufferWriteNTS(&buff, &len, (uint8_t *)"Query Error: Invalid gamename or clientname");
		options->terminated = true;
		send(options->sd,(const char *)buff,len,MSG_NOSIGNAL);
		close(options->sd);
		free((void *)p);
		return;
	}
	if(!options->cryptHeaderSent) {
		setupCryptHeader(options, &buff, &len);
	}
	BufferWriteInt(&buff,&len,options->fromip);
	BufferWriteShort(&buff,&len,0); //default query port, for group listing its always 0
	num_params = countchar((char *)options->fieldlist,'\\');
	BufferWriteShort(&buff,&len,num_params);
	for(int i=0;i<num_params;i++) {
		if(find_param(i, (char *)options->fieldlist, (char *)&field, sizeof(field))) {
			BufferWriteNTS(&buff, &len, (uint8_t *)&field);
			BufferWriteByte(&buff,&len,0); //on gamespys server there is a bug where a 2nd null byte is put after each word here so do it too for compatibility
		}
	}
	sprintf_s(query,sizeof(query),"SELECT `groupid`,`name`,`maxwaiting`,`other` FROM `Gamemaster`.`grouplist` WHERE `gameid` = '%d'",options->queryGame->id);
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		close(options->sd);
		free((void *)p);
		return;
	}
	bool hasMaxServers = options->maxservers != 0;
	res = mysql_store_result(conn);
	while((row = mysql_fetch_row(res)) != NULL && (!hasMaxServers || options->maxservers--)) {
		addGroupBuff((char **)&buff,(int *)&len ,(char *)options->fieldlist,row);
		if(len > MAX_OUTGOING_REQUEST_SIZE) {
			options->cryptHeaderSent = true;
			enctypex_func6e((unsigned char *)&options->encxkeyb,(unsigned char *)p+options->headerLen,len-options->headerLen);
			send(options->sd,(const char *)p,len,MSG_NOSIGNAL);
			options->headerLen = 0;
			len = 0;
			buff = p;
		}
	}
	mysql_free_result(res);
	BufferWriteByte((uint8_t **)&buff,(uint32_t *)&len,0x00);
	BufferWriteInt(&buff,&len,-1);
	options->cryptHeaderSent = true;
	enctypex_func6e((unsigned char *)&options->encxkeyb,(unsigned char *)p+options->headerLen,len-options->headerLen);
	options->headerLen = 0;
	send(options->sd,(const char *)p,len,MSG_NOSIGNAL);
	free((void *)p);
}
void addGroupBuff(char **buff,int *len, char *fieldList, MYSQL_ROW row) {
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
		} else if(strcasecmp(field,"numplayers") == 0) { //TODO: make this the amount of IRC users from peerchat
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
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)"0");
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

}
char *findServerValue(char *name,serverList list) {
	std::list<customKey *>::iterator skeys = list.serverKeys.begin();
	customKey *key;
	while(skeys != list.serverKeys.end()) {
		key = *skeys;
		if(strcmp(key->name,name) == 0) {
			return key->value;
		}
		skeys++;
	}
	return NULL;
}
void addServerBuff(char **buff,int *len,handlerOptions *options, serverList slist) {
	peerchatMsgData peerchatMsg;
	msgNumUsersOnChan *numUsersMsg;
	char field[MAX_FIELD_LIST_LEN + 1],fielddata[MAX_FIELD_LIST_LEN + 1];
	char *fdata;
	int i=0;
	uint8_t flags = 0;
	if((fdata = findServerValue((char *)&field,slist)) != NULL) {
		if(atoi(fdata) != 0) {
			flags |= CONNECT_NEGOTIATE_FLAG;
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
		if(localport != htons(options->queryGame->queryport)) {
			flags |= NONSTANDARD_PRIVATE_PORT_FLAG;
		}
	}
	if(hostport != htons(options->queryGame->queryport)) {
		flags |= NONSTANDARD_PORT_FLAG;
	}
	if((fdata = findServerValue("firewall",slist)) != NULL) {
		if(atoi(fdata) != 0)
			flags |= UNSOLICITED_UDP_FLAG;
	}
	if(options->fieldlist != NULL) flags |= HAS_KEYS_FLAG;
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
	while(find_param(i++, (char *)options->fieldlist, (char *)&field, sizeof(field))) {
		BufferWriteByte((uint8_t **)buff,(uint32_t *)len,0xFF);
		if((fdata = findServerValue((char *)&field,slist)) != NULL) {
			BufferWriteNTS((uint8_t **)buff, (uint32_t *)len, (uint8_t*)fdata);	
		} else {
			BufferWriteByte((uint8_t **)buff,(uint32_t *)len,0x00);
		}
	}
}
void sendServers(handlerOptions *options) {
	uint8_t *buff,*p;
	uint16_t num_params = 0;
	uint32_t len = 0;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char field[MAX_FIELD_LIST_LEN + 1];
	int fi = 0;
	bool sendPacket = false;
	buff = (uint8_t *)malloc(MAX_OUTGOING_REQUEST_SIZE * 2); //* 2 just in case it goes over the buffer size
	if(buff == NULL) return;
	p = buff;
	if(options->game == NULL || options->queryGame == NULL) {
		BufferWriteNTS(&buff, &len, (uint8_t *)"Query Error: Invalid gamename or clientname");
		options->terminated = true;
		send(options->sd,(const char *)buff,len,MSG_NOSIGNAL);
		close(options->sd);
		free((void *)p);
		return;
	}
	if(!options->cryptHeaderSent) {
		setupCryptHeader(options, &buff, &len);
	}
	BufferWriteInt(&buff,&len,options->fromip);
	BufferWriteShortRE(&buff,&len,options->queryGame->queryport); //default query port, for group listing its always 0
	num_params = countchar((char *)options->fieldlist,'\\');
	BufferWriteShort(&buff,&len,num_params);
	for(int i=0;i<num_params;i++) {
		if(find_param(i, (char *)options->fieldlist, (char *)&field, sizeof(field))) {
			BufferWriteNTS(&buff, &len, (uint8_t *)&field);
			BufferWriteByte(&buff,&len,0); //on gamespys server there is a bug where a 2nd null byte is put after each word here so do it too for compatibility
		}
	}
	qrServerMsg msg;
	qrServerList listData;
	msg.data = (void *)&listData;
	listData.game = options->queryGame;
	msg.msgID = EQRMsgID_GetServer;
	servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
	std::list<serverList>::iterator iterator = listData.server_list.begin();
	serverList slist;
	bool hasMaxServers = options->maxservers != 0;
	while(iterator != listData.server_list.end() && (!hasMaxServers || options->maxservers--)) {
		slist = *iterator;
		addServerBuff((char**)&buff,(int *)&len,options,slist);
		if(len > MAX_OUTGOING_REQUEST_SIZE) {
			options->cryptHeaderSent = true;
			enctypex_func6e((unsigned char *)&options->encxkeyb,(unsigned char *)p+options->headerLen,len-options->headerLen);
			send(options->sd,(const char *)p,len,MSG_NOSIGNAL);
			options->headerLen = 0;
			len = 0;
			buff = p;
		}
		iterator++;
	}
	BufferWriteByte((uint8_t **)&buff,(uint32_t *)&len,0x00);
	BufferWriteInt(&buff,&len,-1);
	options->cryptHeaderSent = true;
	enctypex_func6e((unsigned char *)&options->encxkeyb,(unsigned char *)p+options->headerLen,len-options->headerLen);
	options->headerLen = 0;
	send(options->sd,(const char *)p,len,MSG_NOSIGNAL);
	free((void *)p);
}
void setupCryptHeader(handlerOptions *options, uint8_t **dst, uint32_t *len) { 
//	memset(&options->cryptkey,0,sizeof(options->cryptkey));
	srand(time(NULL));
	uint32_t cryptlen = CRYPTCHAL_LEN;
	uint8_t cryptchal[CRYPTCHAL_LEN];
	uint32_t servchallen = SERVCHAL_LEN;
	uint8_t servchal[SERVCHAL_LEN];
	options->headerLen = (servchallen+cryptlen)+(sizeof(uint8_t)*2);
	uint16_t *backendflags = (uint16_t *)(&cryptchal);
	for(uint32_t i=0;i<cryptlen;i++) {
		cryptchal[i] = (uint8_t)rand();		
	}
	*backendflags = options->queryGame->backendflags;
	for(uint32_t i=0;i<servchallen;i++) {
		servchal[i] = (uint8_t)rand();		
	}
	BufferWriteByte(dst,len,cryptlen^0xEC);
	BufferWriteData(dst, len, (uint8_t *)&cryptchal, cryptlen);
	BufferWriteByte(dst,len,servchallen^0xEA);
	BufferWriteData(dst, len, (uint8_t *)&servchal, servchallen);
	enctypex_funcx((unsigned char *)&options->encxkeyb, (unsigned char *)options->game->secretkey,(unsigned char *)options->challenge, (unsigned char *)&servchal,servchallen);
}
void handleInfoRequest(uint8_t *buff, uint32_t len, handlerOptions *handleroptions) {
	uint32_t ip;
	uint16_t port;
	qrServerRules qrRules;
	qrServerMsg msg;
	std::list<customKey *>::iterator it;
	customKey *key;
	buff += 3; //skip the length and request type
	len -= 3;
	ip = BufferReadInt(&buff,&len);
	port = BufferReadShort(&buff,&len);
	qrRules.game = handleroptions->queryGame;
	msg.msgID = EQRMsgID_GetServerRules;
	msg.data = (void *)&qrRules;
	servoptions.sendMsgProc(moduleInfo.name,"qr",(void *)&msg,sizeof(qrServerMsg));
	it = qrRules.server_rules.begin();
	while(it != qrRules.server_rules.end()) {
		key = *it;
		free((void *)key);
		it++;
	}
}
