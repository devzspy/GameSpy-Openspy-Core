#include "main.h"
#include "Client.h"
#include "server.h"
#include "filter.h"
extern serverInfo server;
extern countryRegion countries[];
extern modLoadOptions servoptions;
#ifndef _WIN32
extern GeoIP *gi;
#endif
/*
TODO: for legacy queries the default port is the udp source port, but the 1st parameters value is also the source port, add code to check if they are different?
*/
Client::Client(int sd, struct sockaddr_in *peer) {
	this->sd = sd;
	memcpy(&this->sockinfo,peer,sizeof(struct sockaddr_in));	
	memcpy(&this->servinfo,peer,sizeof(struct sockaddr_in));
	connecttime = time(NULL);
	sentChallenge = false;
	memset(&instancekey,0,sizeof(instancekey));
	hasInstanceKey = false;
	game = NULL;
	serverRegistered = false;
	char *code = NULL;
	#ifndef _WIN32
	if(gi != NULL)
		code = ((char *)GeoIP_country_code_by_ipnum(gi,reverse_endian32(getAddress())));
	#endif
	country = findCountryByName(code);
	legacyQuery = false;
}
void Client::handleLegacyIncoming(char *buff, int len) {
	char type[128];
	if(!find_param(0,buff,(char *)&type,sizeof(type))) {
		return; //no type
	}
	/*if(strcmp(type,"heartbeat") == 0)*/
	if(strcmp(type,"validate") == 0) {
	}
	else if(strcmp(type,"echo") == 0) {
	
	} else {  //heartbeat request or response from status keys
		handleLegacyHeartbeat(buff,len);
	}
}
void Client::handleIncoming(char *buff, int len) {
	lastPing = time(NULL);//treat anything as a ping response
	if(len < 3) {
		deleteClient(this);
	}
	if(!legacyQuery && buff[0] == '\\') {
		bool success = true;
		for(int i=1;i<sizeof(uint32_t) + 1;i++) {
			if(!is_loweralpha(buff[i])) {
				success = false;
				break;
			}
		}
		legacyQuery = true;
	}
	//its assumed you won't touch the values after this(they will be changed by whatever function because of bufferread/write)
	if(legacyQuery) {
		handleLegacyIncoming(buff,len);
		return;
	}
	uint8_t type = BufferReadByte((uint8_t**)&buff,(uint32_t *)&len);
	if(hasInstanceKey) { //spoofed packet check
		for(int i=0;i<sizeof(instancekey);i++) {
			if(instancekey[i] != BufferReadByte((uint8_t**)&buff,(uint32_t *)&len)) {
				return;
			}
		}
	}
	switch(type) {
		case PACKET_HEARTBEAT: {
			handleHeartbeat(buff,len);
			break;
		}
		case PACKET_AVAILABLE: {
			handleAvailable(buff,len);
			break;
		}
		case PACKET_CHALLENGE: { //challenge reply, register server if valid
			handleChallenge(buff,len);
			break;
		}
		case PACKET_KEEPALIVE: {
			handleKeepalive(buff,len);
			break;
		}
	}
}
void Client::handleKeepalive(char *buff,int len) {
	char buffer[256];
	int blen = 0;
	char *p = (char *)&buffer;
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_1);
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_2);
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,PACKET_KEEPALIVE);
	BufferWriteData((uint8_t **)&p, (uint32_t *)&blen, (uint8_t *)&instancekey, sizeof(instancekey));
	sendto(sd,(char *)&buffer,blen,0,(struct sockaddr *)&sockinfo, sizeof(sockaddr_in));
}
void Client::setData(char *variable, char *value) {
	customKey *key;
	if(strcmp(variable,"gamename") == 0) {
		if(game == NULL) 
			game = servoptions.gameInfoNameProc(value);
		return;
	}
	key = findKey(variable);
	if(key == NULL) {
		key = (customKey *)malloc(sizeof(customKey));
		memset(key,0,sizeof(customKey));
		serverKeys.push_back(key);
	}
	if(key->name == NULL) {
		key->name = (char *)calloc(strlen(variable)+1,1);
		if(key->name != NULL) {
			strcpy(key->name,variable);
		}
	}
	if(key->value != NULL) {
		free((void *)key->value);
	} 
	key->value = (char *)calloc(strlen(value)+1,1);
	if(key->value != NULL) {
		strcpy(key->value,value);
	}
}
void Client::handleServerData(char *buff, int len) {
	char variable[128],value[128];
	int i=0;
	clearKeys();
	while(find_param(i++,buff,(char *)&variable,sizeof(variable))) {
		if(find_param(i++,buff,(char *)&value,sizeof(value))) {
			setData((char *)&variable,(char *)&value);
		}
	}
	if(getStateChanged() == 2) {
		deleteClient(this);
		return;
	}
}
void Client::handleLegacyHeartbeat(char *buff,int len) {
	int i=0;
	handleServerData(buff,len);
	if(!serverRegistered) {
		//there is no validating, atleast not anymore because gamespy removed it for compatibility with the new protocol
//		pushServer();
		serverRegistered = true;
	}
	uint32_t blen = 0;
	uint8_t buffer[24];
	uint8_t *p = (uint8_t *)&buffer;
	BufferWriteNTS((uint8_t**)&p,(uint32_t *)&blen,(uint8_t *)"\\status\\");
	blen--; //we don't want the null byte
	sendto(sd,(char *)&buffer,blen,0,(struct sockaddr *)&sockinfo, sizeof(sockaddr_in));
}
void Client::handleHeartbeat(char *buff, int len) {
	char buffer[MAX_DATA_SIZE];
	char *p = (char *)&buffer;
	int blen = 0;
	if(!hasInstanceKey) { //XXX: is this right? hasn't the data been skipped in the check? nvm because if hasInstanceKey is false it won't try read the instance key
		for(int i=0;i<sizeof(instancekey);i++) {
			instancekey[i] = BufferReadByte((uint8_t**)&buff,(uint32_t *)&len);
		}
		hasInstanceKey = true;
	}
	int i = 0;
	customKey *key = NULL;
	clearKeys();
	uint8_t *x;
	while((buff[0] != 0 && len > 0) || (i%2 != 0)) {
		x = BufferReadNTS((uint8_t **)&buff,(uint32_t *)&len);
		if(i%2 == 0) {
			key = (customKey *)calloc(sizeof(customKey),1);
			key->name = (char *)calloc(strlen((char *)x)+1,1);
			strcpy(key->name,(char *)x);
		} else {
			if(key != NULL) {
				key->value = (char *)calloc(strlen((char *)x)+1,1);
				strcpy(key->value,(char *)x);
				serverKeys.push_back(key);
			}
		}
		free((void *)x);
		i++;
	}
	uint16_t num_values = 0;
	BufferReadByte((uint8_t**)&buff,(uint32_t *)&len); //skip null byte(seperator)
	while((num_values = BufferReadShortRE((uint8_t**)&buff,(uint32_t *)&len))) {
		std::deque<uint8_t *> nameValueList;
		if(len > 3) { //player or team keys
			uint32_t num_keys = 0;
			while(buff[0] != 0 && len > 0) {
				x = BufferReadNTS((uint8_t **)&buff,(uint32_t *)&len);	
				nameValueList.push_back(x);
				num_keys++;
				if(buff[0] == 0) break;
				//free((void *)x);
			}
			int i=0,player=0,num_keys_t = num_keys,num_values_t = num_values*num_keys;
			BufferReadByte((uint8_t**)&buff,(uint32_t *)&len);
			while(num_values_t--) {
			//while(buff[0] != 0 && len > 0) {
				bool is_team;
				x = BufferReadNTS((uint8_t **)&buff,(uint32_t *)&len);	
				//BufferReadByte((uint8_t**)&buff,(uint32_t *)&len); //skip null byte
				indexedKey *key = (indexedKey *)malloc(sizeof(indexedKey));
				char *value = NULL;
				if(key != NULL) {
					char *name = (char *)nameValueList.at(i);
					if(name != NULL) {
					is_team = isTeamString((char *)name);
					value = (char *)x;
					key->key.name = (char *)calloc(strlen(name) +1,1);
					strcpy(key->key.name,name);
					} else key->key.name = NULL;
					if(value != NULL) {
						key->key.value = (char *)calloc(strlen(value)+1,1);
						strcpy(key->key.value,value);
					} else key->key.value = NULL;
					key->index = player;
				}
				if(is_team) {
					teamKeys.push_back(key);
				} else {
					playerKeys.push_back(key);
				}
				free((void *)x);
				i++;
				if(i == num_keys) {
					player++;
					i=0;
				}
			}
		}
		std::deque<uint8_t *>::iterator iterator = nameValueList.begin();
		while(iterator != nameValueList.end()) {
			free((void *)*iterator);
			iterator++;
		}
		nameValueList.clear();
	}
	if(getGameInfo() == NULL) {
		game = servoptions.gameInfoNameProc(findServerValue("gamename"));
		if(game == NULL) { 
			deleteClient(this);
			return;
		}
	}
	if(game->servicesdisabled != 0) {
		deleteClient(this);
		return;
	}
	if(getStateChanged() == 2) {
		deleteClient(this);
		return;
	}
	if(!sentChallenge) {
		gen_random((char *)&challenge,sizeof(challenge));
		BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_1);
		BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_2);
		BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,PACKET_CHALLENGE);
		BufferWriteData((uint8_t **)&p, (uint32_t *)&blen, (uint8_t *)&instancekey, sizeof(instancekey));
		BufferWriteNTS((uint8_t**)&p,(uint32_t *)&blen,(uint8_t *)&challenge);
		sendto(sd,(char *)&buffer,blen,0,(struct sockaddr *)&sockinfo, sizeof(sockaddr_in));
		sentChallenge = true;
	}
}
void Client::handleAvailable(char *buff, int len) {
	char buffer[MAX_DATA_SIZE];
	char *p = (char *)&buffer;
	int blen = 0;
	uint8_t disabledservices = 0; //available
	BufferReadInt((uint8_t**)&buff,(uint32_t *)&len); //skip the instance ID
	gameInfo *game = servoptions.gameInfoNameProc(buff);
	if(game != NULL) {
		disabledservices = game->servicesdisabled;
	}
	//printf("availablecheck: %s %p %d\n",game!=NULL?game->name:NULL,game,disabledservices);
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_1);
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_2);
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,PACKET_AVAILABLE);
	BufferWriteIntRE((uint8_t**)&p,(uint32_t *)&blen,0); //instance key aka nothing
	BufferWriteIntRE((uint8_t**)&p,(uint32_t *)&blen,disabledservices);
	sendto(sd,(char *)&buffer,blen,0,(struct sockaddr *)&sockinfo, sizeof(sockaddr_in));
}
void Client::handleChallenge(char *buff, int len) {
	char challenge[90];
	char buffer[MAX_DATA_SIZE];
	char *p = (char *)&buffer;
	int blen = 0;
	gameInfo *game = NULL;
	game = getGameInfo();
	if(game == NULL) return;
	gsseckey((unsigned char *)&challenge, (unsigned char *)&this->challenge, (unsigned char *)game->secretkey, 0);
	if(strcmp(buff,challenge) == 0) { //matching challenge
		BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_1);
		BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_2);
		BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,PACKET_CLIENT_REGISTERED);
		BufferWriteData((uint8_t **)&p, (uint32_t *)&blen, (uint8_t *)&instancekey, sizeof(instancekey));
		sendto(sd,(char *)&buffer,blen,0,(struct sockaddr *)&sockinfo, sizeof(sockaddr_in));	
		if(serverRegistered == false) {
			serverRegistered = true;
		}
	}
	if(serverRegistered) {
		pushServer();
	}
}
Client::~Client() {
	pushDelete();
	clearKeys();
}
struct sockaddr_in *Client::getSockAddr() {
	return (struct sockaddr_in *)&sockinfo;
}
time_t Client::getLastPing() {
	return lastPing;
}
void Client::clearKeys() {
	customKey *key;
	while(!serverKeys.empty()) {
		key = serverKeys.front();
		if(key->value != NULL) free((void *)key->value);
		if(key->name != NULL) free((void *)key->name);
		if(key != NULL) free((void *)key);
		serverKeys.pop_front();
	}
	indexedKey *pkey;
	while(!playerKeys.empty()) {
		pkey = playerKeys.front();
		if(pkey->key.value != NULL) free((void *)pkey->key.value);
		if(pkey->key.name != NULL) free((void *)pkey->key.name);
		free((void *)pkey);
		playerKeys.pop_front();
	}
	while(!teamKeys.empty()) {
		pkey = teamKeys.front();
		if(pkey->key.value != NULL) free((void *)pkey->key.value);
		if(pkey->key.name != NULL) free((void *)pkey->key.name);
		free((void *)pkey);
		teamKeys.pop_front();
	}
	
}
customKey *Client::findKey(char *name) {
	customKey *key;
	std::list<customKey *>::iterator iterator = serverKeys.begin();
	while(iterator != serverKeys.end()) {
		key = *iterator;
		if(key->name != NULL) {
			if(strcmp(key->name,name) == 0) {
				return key;
			}
		}
		iterator++;
	}
	return NULL;
}
char *Client::findServerValue(char *name) {
	static char regionbuff[16];
	std::list<customKey *>::iterator iterator;
	iterator=serverKeys.begin();
	customKey *key;
	if(stricmp(name,"country") == 0) {
		return (char *)country->countrycode;
	} else if(stricmp(name,"region") == 0) {
		sprintf(regionbuff,"%d",country->region);
		return (char *)&regionbuff; //kinda dangerous
	} else if(stricmp(name,"gamename") == 0) {
		if(getGameInfo() != NULL) {
			return getGameInfo()->name;
		}
	}
	while(iterator != serverKeys.end()) {
		key = *iterator;
		if(key->name != NULL) {
			if(strcmp(key->name,name) == 0) {
				return key->value;
			}
		}
		iterator++;
	}
	return NULL;
}
bool Client::isTeamString(char *string) {
	int len = strlen(string);
	if(string[len-2] == '_' && string[len-1] == 't') {
		return true;
	}
	return false;
}
std::list<customKey *> Client::getServerKeys() {
	return serverKeys;
}
std::list<customKey *> Client::getRules() {
	std::list<customKey *> rules;
	std::list<customKey *>::iterator iterator;
	std::list<indexedKey *>::iterator it2;
	iterator=serverKeys.begin();
	customKey *key,*key2;
	indexedKey *ikey;
	while(iterator != serverKeys.end()) {
		key = *iterator;
		if(key == NULL || key->name == NULL || key->value == NULL) continue;
		key2 = (customKey *)malloc(sizeof(customKey));
		key2->name = (char *)calloc(strlen(key->name)+1,1);
		strcpy(key2->name,key->name);
		key2->value = (char *)calloc(strlen(key->value)+1,1);
		strcpy(key2->value,key->value);
		rules.push_back(key2);
		iterator++;
	}
	it2 = playerKeys.begin();
	while(it2 != playerKeys.end()) {
		ikey = *it2;
		key = (customKey *)malloc(sizeof(customKey));
		if(ikey == NULL || ikey->key.name == NULL || ikey->key.value == NULL) continue;
		key->name = (char *)calloc(strlen(ikey->key.name)+32,1);
		key->value = (char *)calloc(strlen(ikey->key.value)+32,1);
		sprintf(key->name,"%s%d",ikey->key.name,ikey->index);
		strcpy(key->value,ikey->key.value);
		rules.push_back(key);
		it2++;
	}
	it2 = teamKeys.begin();
	while(it2 != teamKeys.end()) {
		ikey = *it2;
		key = (customKey *)malloc(sizeof(customKey));
		key->name = (char *)calloc(strlen(ikey->key.name)+32,1);
		key->value = (char *)calloc(strlen(ikey->key.value)+32,1);
		sprintf(key->name,"%s%d",ikey->key.name,ikey->index);
		strcpy(key->value,ikey->key.value);
		rules.push_back(key);
		it2++;
	}
	return rules;
}
std::list<customKey *> Client::copyServerKeys() {
	std::list<customKey *> rules;
	std::list<customKey *>::iterator iterator;
	iterator=serverKeys.begin();
	customKey *key,*key2;
	while(iterator != serverKeys.end()) {
		key = *iterator;
		if(key != NULL) {
			key2 = (customKey *)malloc(sizeof(customKey));
			if(key->name != NULL) {
				key2->name = (char *)calloc(strlen(key->name)+1,1);
				strcpy(key2->name,key->name);
			}
			if(key->value != NULL) {
				key2->value = (char *)calloc(strlen(key->value)+1,1);
				strcpy(key2->value,key->value);
			}
		}
		rules.push_back(key2);
		iterator++;
	}
	return rules;
}
gameInfo *Client::getGameInfo() {
	return game;
}
uint16_t Client::getPort() {
	char *sayport = findServerValue("heartbeat");
	if(sayport != NULL) {
		return htons(atoi(sayport));
	}
	return sockinfo.sin_port;
}
uint32_t Client::getStateChanged() {
	char *state = findServerValue("statechanged");
	if(state != NULL) {
		return atoi(state);
	}
	return 0;
}
uint32_t Client::getAddress() {
	return sockinfo.sin_addr.s_addr;
}
uint32_t Client::getServerAddress() {
/*
	uint32_t retip;
	char *ipstr = findServerValue("publicip");
	if(ipstr != NULL) {
		retip = atoi(ipstr);
		retip = htonl(retip);
		return retip;
	}
*/
	return sockinfo.sin_addr.s_addr;
}
uint32_t Client::getServerPort() {
	uint16_t retip;
	char *ipstr = findServerValue("hostport");
	if(ipstr != NULL) {
		retip = atoi(ipstr);
		retip = htons(retip);
		return retip;
	}
	return sockinfo.sin_port;
}
countryRegion *Client::getCountry() {
	return country;
}
void Client::pushServer() {
	sbServerMsg msg;
	sbPushMsg sbMsg;
	msg.msgID = ESBMsgID_PushServer;
	msg.data = (void *)&sbMsg;
	sbMsg.ipaddr = getServerAddress();
	sbMsg.port = getServerPort();
	sbMsg.keys = getServerKeys();
	sbMsg.country = getCountry();
	sbMsg.game = getGameInfo();
	if(sbMsg.game == NULL) return;	
	servoptions.sendMsgProc(moduleInfo.name,"serverbrowsing",(void *)&msg,sizeof(sbServerMsg));
}
void Client::pushDelete() {
	sbServerMsg msg;
	sbPushMsg sbMsg;
	msg.msgID = ESBMsgID_DeleteServer;
	msg.data = (void *)&sbMsg;
	sbMsg.ipaddr = getServerAddress();
	sbMsg.port = getServerPort();
	sbMsg.keys = getServerKeys();
	sbMsg.country = getCountry();
	sbMsg.game = getGameInfo();
	if(sbMsg.game == NULL) return;	
	servoptions.sendMsgProc(moduleInfo.name,"serverbrowsing",(void *)&msg,sizeof(sbServerMsg));
}
void Client::sendMsg(void *data, int len) {
	uint8_t senddata[256];
	uint32_t blen = 0;
	uint8_t *test = (uint8_t *)data;
	uint8_t *p = (uint8_t *)&senddata;
	if(len > sizeof(senddata) -1) return;
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_1);
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,QR_MAGIC_2);
	BufferWriteByte((uint8_t**)&p,(uint32_t *)&blen,PACKET_CLIENT_MESSAGE);
	BufferWriteData((uint8_t **)&p, (uint32_t *)&blen, (uint8_t *)&instancekey, sizeof(instancekey));
	uint32_t msgKey = rand();
	BufferWriteInt((uint8_t**)&p,(uint32_t *)&blen,msgKey);
	BufferWriteData((uint8_t **)&p, (uint32_t *)&blen, (uint8_t *)data, len);
	sendto(sd,(char *)&senddata,blen,0,(struct sockaddr *)&sockinfo, sizeof(sockaddr_in));	
}
bool Client::isServerRegistered() {
	return serverRegistered;
}
