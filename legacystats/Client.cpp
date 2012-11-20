#include <common/chc_endian.h>
#include "Client.h"
#include <legacystatsprocessor/structs.h>
extern legacyStatsServer server;
extern modInfo moduleInfo;
Client::Client(clientParams* params) {
	sd = params->sd;
	profileid = 0;
	userid = 0;
	game = NULL;
	authenticated = false;
	ip = params->peer.sin_addr.s_addr;
	lastPacket = time(NULL);
	sesskey = time(NULL) * rand();
	if(sesskey < 0) sesskey = -sesskey;
	memset(&challenge,0,sizeof(challenge));
	gen_random(challenge,10);
	gameInProgress = false;
	formatSend(sd,true,2,"\\lc\\1\\challenge\\%s\\id\\1",challenge);
	free((void *)params);
}
Client::~Client() {
	if(gameInProgress) {
		saveGame(false);
		clearGameKeys();
	}
	close(sd);
}
int Client::getSocket() {
	return sd;
}
void Client::mainLoop(fd_set *rset) {
	if(!FD_ISSET(sd,rset)) return;
	memset(&buff,0,sizeof(buff));
	len = recv(sd,buff,sizeof(buff),0);
	if(len<1) goto end;
	lastPacket = time(NULL);
	if(!do_db_check()) return
	gamespy3dxor(buff, len);
	buff[len]=0;
	makeStringSafe((char *)&buff, sizeof(buff));
	parseIncoming();
	return;
end:
	 deleteClient(this);
}
void Client::sendError(int sd, bool fatal, char *msg, GPErrorCode errid, int id) {
	int len = 128 + strlen(msg);
	char *errtext = (char *)malloc(len);
	sprintf_s(errtext,len,"\\error\\\\err\\%d",errid);
	if(fatal) {
		strcat(errtext,"\\fatal\\");
	}
	if(msg != NULL) {
		strcat(errtext,"\\errmsg\\");
		strcat(errtext,msg);
//		if(strchr(msg,'\\') == NULL)
//			strcat(errtext,"\\");
	}
	if(id == -1) {
		formatSend(sd,true,2,"%s",errtext);
	} else {
		formatSend(sd,true,2,"%s\\id\\%d",errtext,id);
	}
	free((void *)errtext);
	if(fatal) {
		deleteClient(this);
	}
	return;
}
void Client::parseIncoming() {
	char *p = (char *)&buff;	
	char *x;
	while(true) {
		x = p;
		p = strstr(p,"\\final\\");
		if(p == NULL) { break; }
		*(p+1) = 0;
		p+=7;
		parseIncoming((char *)x,strlen(x));
	}
	if(strlen(x) > 7 && (x - (char *)&buff) < len) {
		parseIncoming((char *)x,strlen(x));
	}
}
void Client::parseIncoming(char *buff, int len) {
	char type[128];
	char *next;
	if(!find_param(0, buff, type,sizeof(type))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;	
	}
//	printf("msg type: %s\n",type);
//	if(userid == 0) {
//		sendError(sd,true,"This request cannot be processed because you are not logged in.",GP_NOT_LOGGED_IN,-1);
//		deleteClient(this);
//		return;
//	}
	if(strcmp(type,"auth") == 0) {
		handleAuth(buff,len);
	} else if(strcmp(type,"ka") == 0) {
		
	}
	if(authenticated == 0 || game == NULL) {
		sendError(sd,true,"Failed to validate",GP_GENERAL,1);
		return;
	}
	if(strcmp(type,"authp") == 0) {
		handleAuthP(buff,len);
	} else if(strcmp(type,"newgame") == 0) {
		gameInProgress = true;
	} else if(strcmp(type,"updgame") == 0) {
		handleUpdateGame(buff,len);
	} else if(strcmp(type,"setpd") == 0) {
		handleSetPD(buff,len);
	} else if(strcmp(type,"getpd") == 0) {
		handleGetPD(buff,len);
	} else {
//		printf("Unknown Command: %s\n",buff);
		return;
	}
}
void Client::clearGameKeys() {
	std::list<gameKey *>::iterator it = gameKeys.begin();
	gameKey *key;
	while(it != gameKeys.end()) {
		key = *it;
		if(key->name != NULL) {
			free((void *)key->name);
		}
		if(key->value != NULL) {
			free((void *)key->value);
		}
		free((void *)key);
		it++;
	}
	gameKeys.clear();
}
void Client::handleUpdateGame(char *buff,int len) {
	char gamedata[1024];
	int done = find_paramint("done", buff);
	if(!find_param("gamedata",buff,gamedata,sizeof(gamedata))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);	
		return;
	}
	for(int i=0;i<strlen(gamedata);i++) {
		if(gamedata[i] == '\x1') {
			gamedata[i] = '\\';
		}
	}
	int i=0;
	char sbuff[1024],value[1024];
	gameKey *key = NULL;
	clearGameKeys();
	while(find_param(i++,gamedata,sbuff,sizeof(sbuff))) {
		key = (gameKey *)malloc(sizeof(gameKey));
		if(key == NULL) {
			break;
		}
		int xlen = strlen(sbuff) + 1;
		key->name = (char *)malloc(xlen);
		if(key->name == NULL) {
			break;
		}
		sprintf_s(key->name, xlen,"%s",sbuff);
		if(find_param(i++,gamedata,value,sizeof(value))) {
			int mlen = strlen(value) + 1;
			key->value = (char *)malloc(mlen);
			sprintf_s(key->value,mlen,"%s",value);
		}
		gameKeys.push_back(key);
	}
	if(done) {
		saveGame(done);
		gameInProgress = false;
		clearGameKeys();
	}
	
}
int Client::getKeyStrSize() {
	int len = 0;
	std::list<gameKey *>::iterator it = gameKeys.begin();
	gameKey *key;
	while(it != gameKeys.end()) {
		key = *it;
		if(key->name != NULL) {
			len += strlen(key->name) + 1;
		}
		if(key->value != NULL) {
			len += strlen(key->value) + 1;
		}
		it++;
	}
	return len + 1;
}
void Client::saveGame(bool done) {
	int mlen = (getKeyStrSize() * 2) + 1;
	char *savedata = (char *)malloc(mlen);
	if(savedata == NULL) return;
	memset(savedata,0,mlen);
	int len = 0;
	std::list<gameKey *>::iterator it = gameKeys.begin();
	gameKey *key;
	while(it != gameKeys.end()) {
		key = *it;
		strcat(savedata,"\\");
		if(key->name != NULL) {
			strcat(savedata,key->name);
		}
		strcat(savedata,"\\");
		if(key->name != NULL) {
			strcat(savedata,key->value);
		}
		it++;
	}
	//char *query = (char *)malloc(mlen + 256);
	LSPBaseMsg spmsg;
	LSPSnapshotMsg omsg;
	spmsg.msgID = ELSPMsgID_ProcessSnapshot;
	spmsg.data = (void *)&omsg;
	spmsg.len = sizeof(LSPSnapshotMsg);
	omsg.data = savedata;
	omsg.profileid = profileid;
	omsg.gameid = game->id;
	omsg.done = done;
	server.options->sendMsgProc(moduleInfo.name,"legacystatsprocessor",(void *)&spmsg,sizeof(LSPBaseMsg));
	/*
	for(int i=0;i<mlen;i++) {
		if(savedata[i] == '\\') {
			savedata[i] = '\x1';
		}
	}
	mysql_real_escape_string(server.conn,savedata,savedata,strlen(savedata));
	sprintf_s(query,mlen + 256,"INSERT INTO `Persist`.`snapshots` (`gameid`,`profileid`,`keys`,`done`) VALUES (%d,%d,\"%s\",%d)",game->id,profileid,savedata,done);
	if(strlen(savedata) > 0) {
		if(mysql_query(server.conn,query)) {
			fprintf(stderr,"%s\n",mysql_error(server.conn));
		}
	}
	*/
	//free((void *)query);
	free((void *)savedata);	
}
void Client::handleAuth(char *buff,int len) {
	char response[33],response2[33];
	char gamename[64];
	char keyhash[33];
	memset(&response,0,sizeof(response));
	memset(&response2,0,sizeof(response2));
	if(!find_param("response", buff, response, sizeof(response))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	memset(&gamename,0,sizeof(gamename));
	if(!find_param("gamename", buff, gamename, sizeof(gamename))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	if(game == NULL) {
		game = server.options->gameInfoNameProc(gamename);
	}
	if(game == NULL) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	int chrespnum = gs_chresp_num(challenge);
	getResponse(chrespnum,game->secretkey,response2,sizeof(response2));
	if(strcmp(response2,response) != 0) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	authenticated = true;
	formatSend(sd,true,2,"\\lc\\2\\sesskey\\%d\\proof\\0\\id\\1",sesskey);
}
int Client::tryCdKeyLogin(char *keyhash) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[256];
	int pid;
	int clen = (strlen(keyhash)*2)+1;
	char *keyhashesc = (char *)malloc(clen);
	if(keyhashesc == NULL) return false;
	mysql_real_escape_string(server.conn,keyhashesc,keyhash,strlen(keyhash));
	sprintf_s(query,sizeof(query),"SELECT `profileid` FROM `GameTracker`.`authedcdkeys` WHERE md5(concat(\"%s\",cdkey)) = \"%s\" AND gameid = %d",challenge,keyhashesc,game->id);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return 0;
	}
	res = mysql_store_result(server.conn);
   	while ((row = mysql_fetch_row(res)) != NULL) {
		pid = atoi(row[0]);
	}
	mysql_free_result(res);
	free((void *)keyhashesc);
	return pid;
}
void Client::handleAuthP(char *buff,int len) {
	char response[33];
	int pid = find_paramint("pid",buff);
	int lid = find_paramint("lid",buff);
	/*
	if(pid == 0) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	*/
	if(!find_param("resp", buff, response, sizeof(response))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	int userid = getProfileUserID(server.conn,pid);
	if(userid == 0) {
		formatSend(sd,true,2,"\\pauthr\\-1\\lid\\%d\\",lid);
		return;
	}
	char pass[128];
	char keyhash[33];
	bool authed = false;
	if(find_param("keyhash", buff, keyhash, sizeof(keyhash))) {
		pid = tryCdKeyLogin(keyhash);
		if(pid != 0) {
			userid = getProfileUserID(server.conn,pid);
			authed = true;
		}	
	}
	else {
		getProfileIDPass(server.conn, pid, pass, sizeof(pass));
		getLoginResponse(gs_sesskey(sesskey), pass, pass, sizeof(pass));
		authed = strcmp(response,pass) == 0;
	}
	if(authed) {
		this->userid = userid;
		this->profileid = pid;
		formatSend(sd,true,2,"\\pauthr\\%d\\lid\\%d\\",pid,lid);
	} else {
		formatSend(sd,true,2,"\\pauthr\\-1\\lid\\%d\\",lid);
	}
}
void Client::handleGetPD(char *buff,int len) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	int pid = find_paramint("pid",buff);	
	persisttype_t type = (persisttype_t) find_paramint("ptype",buff);
	if(type == pd_private_ro || type == pd_private_rw) {
		if(pid != getProfileID()) {
			formatSend(sd,true,2,"\\getpdr\\0\\lid\\0\\pid\\%d\\errmsg\\No authentication",pid);
			return;
		}
	}
	int index = find_paramint("dindex",buff);
	//the 1 = success
	char query[256];
	char kstr[256];
	if(!find_param("keys",buff,kstr,sizeof(kstr))) {
		//sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		//return;	
	} else {
		for(int x=0;x<strlen(kstr);x++) {
			if(kstr[x] == '\x01') {
				kstr[x] = '\\';
			}
		}	
	}
	sprintf_s(query,sizeof(query),"SELECT `data`,unix_timestamp(`modified`) FROM `Persist`.`data` WHERE `gameid` = %d AND `index` = %d AND `type` = %d AND `profileid` = %d",game->id,index,type,pid);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	res = mysql_store_result(server.conn);
	if(mysql_num_rows(res) < 1) {
		formatSend(sd,true,2,"\\getpdr\\1\\lid\\0\\pid\\%d\\length\\%d\\data\\",pid,0);
	} else {
		row = mysql_fetch_row(res);
		int mlen = 1024;
		char *sendstr = NULL;
		int i=0;
		char buff2[256];
		int modified = atoi(row[1]);
		if(strlen(kstr) < 1) {
			mlen = strlen(kstr) + 1;
			sendstr = (char *)malloc(mlen);
			memset(sendstr,0,mlen);
			strcat(sendstr,row[0]);
		} else {
			sendstr = (char *)malloc(mlen);
			memset(sendstr,0,mlen);
			while(find_param(i++,kstr,buff2,sizeof(buff2))) {
				char buff3[256];
//				printf("%s is kstr\n",buff2);
//				memset(&buff2,0,sizeof(buff2));
				if(find_param((char *)&buff2,row[0],(char *)&buff3,sizeof(buff3))) {
					strcat(sendstr,"\\");
					strcat(sendstr,buff2);
					strcat(sendstr,"\\");
					if((strlen(buff2) + strlen(sendstr) + strlen(buff3)) > (mlen-256)) {
						mlen += strlen(buff2) + strlen(buff3) + 1024;
						sendstr = (char *)realloc(sendstr,mlen);
					}
					strcat(sendstr,buff3);
				}
			}
		}
		formatSend(sd,true,2,"\\getpdr\\1\\lid\\0\\pid\\%d\\mod\\%d\\length\\%d\\data\\%s",pid,modified,strlen(sendstr),sendstr);
		free((void *)sendstr);
	}
	mysql_free_result(res);
	
}
bool Client::shouldUpdateData(int dindex, persisttype_t type, int pid) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[256];
	sprintf_s(query,sizeof(query),"SELECT 1 FROM `Persist`.`data` WHERE `profileid` = %d AND `type` = %d AND `index` = %d  AND `gameid` = %d",pid,type,dindex,game->id);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return false;
	}
	res = mysql_store_result(server.conn);
	bool shouldUpdate = mysql_num_rows(res) > 0;
	mysql_free_result(res);
	return shouldUpdate;
}
char *Client::mergeKeys(char *keys,int localid,persisttype_t type,int index,int pid) {
	char *curkeys;
	char *rkeys = NULL;
	char query[256];
	MYSQL_RES *res;
	MYSQL_ROW row;
	sprintf_s(query,sizeof(query),"SELECT `data` FROM `Persist`.`data` WHERE `profileid` = %d AND `index` = %d AND `type` = %d",pid,index,type);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return keys;
	}
	res = mysql_store_result(server.conn);
	while ((row = mysql_fetch_row(res)) != NULL) {
		curkeys = row[0];
		break;
	}
	int mlen = strlen(keys) + strlen(curkeys) + 1;
	rkeys = (char *)malloc(mlen);
	if(rkeys == NULL) {
		return keys;
	}
	memset(rkeys,0,mlen);
	int i = 0;
	char sbuff[1024],kbuff[1024];
	while(find_param(i,curkeys,sbuff,sizeof(sbuff))) {
		if(!find_param(sbuff,keys,kbuff,sizeof(kbuff))) {
			if(find_param(sbuff,curkeys,kbuff,sizeof(kbuff))) {
				if(strlen(kbuff) > 0) {
					strcat(rkeys,"\\");
					strcat(rkeys,sbuff);
					strcat(rkeys,"\\");
					strcat(rkeys,kbuff);
				}
			}
		} else {
			if(find_param(i+1,curkeys,kbuff,sizeof(kbuff))) {
				if(strlen(kbuff) > 0) {
					strcat(rkeys,"\\");
					strcat(rkeys,sbuff);
					strcat(rkeys,"\\");
					strcat(rkeys,kbuff);
				}
			}
		}
		i+=2;
	}
	mysql_free_result(res);
	return rkeys;
}
void Client::handleSetPD(char *buff,int len) {
	int pid = find_paramint("pid",buff);
	int kvset = find_paramint("kv",buff);
	int dindex = find_paramint("dindex",buff);
	int length = find_paramint("length",buff);
	int localid = find_paramint("lid",buff);
	persisttype_t type = (persisttype_t) find_paramint("ptype", buff);
	if(pid != this->profileid || this->profileid == 0) {
		formatSend(sd,true,2,"\\setpdr\\0\\lid\\%d\\pid\\%d\\errmsg\\No authentication",localid,pid);
		return;
	}
	if(type == pd_private_ro || type == pd_public_ro) {
		formatSend(sd,true,2,"\\setpdr\\0\\lid\\%d\\pid\\%d\\errmsg\\Read-Only data",localid,pid);
		return;
	}
	bool update = shouldUpdateData(dindex,type,pid);
	char *savedata = strstr(buff,"\\data\\");
	if(savedata == NULL || length < 0) {
		formatSend(sd,true,2,"\\setpdr\\0\\lid\\%d\\pid\\%d\\errmsg\\Missing data",localid,pid);
	}
	if(length > strlen(savedata)) { //incase they specifiy a length too large to read stack space, etc
		formatSend(sd,true,2,"\\setpdr\\0\\lid\\%d\\pid\\%d\\errmsg\\Invalid length",localid,pid);
		return;
	}
	char *finalstr = strstr(buff,"\\final\\");
	if(finalstr != NULL) {
		*finalstr = 0;
	}
	savedata += 6; //strlen of \\data\\ --
	if(length < strlen(savedata)) {
		savedata[length] = 0;
	}
	if(update && kvset) {
		savedata = mergeKeys(savedata,localid,type,dindex,pid);
	}
	int mlen = (strlen(savedata) * 2) + 1;
	int slen = mlen + 256;
	char *escapestr = (char *)malloc(mlen);
	memset(escapestr,0,mlen);
	mysql_real_escape_string(server.conn,escapestr,savedata,strlen(savedata));
	char *querystr = (char *)malloc(slen);
	if(update) {
	sprintf_s(querystr,slen, "UPDATE `Persist`.`data` SET `data` = \"%s\", `modified` = CURRENT_TIMESTAMP WHERE `profileid` = %d AND `index` = %d AND `type` = %d AND `gameid` = %d",escapestr,pid,dindex,type,game->id);
	} else {
	sprintf_s(querystr,slen , "INSERT INTO `Persist`.`data` (`data`,`profileid`,`index`,`type`,`gameid`) VALUES (\"%s\",%d,%d,%d,%d)",escapestr,pid,dindex,type,game->id);
	}
	if(mysql_query(server.conn,querystr)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
	}
	formatSend(sd,true,2,"\\setpdr\\1\\lid\\%d\\pid\\%d\\mod\\%d",localid,pid,time(NULL));
	if(update && kvset) {
		free((void *)savedata);
	}
	free((void *)querystr);
	free((void *)escapestr);
}
void Client::getLoginResponse(char *sesskey, char *pass, char *out, int dstlen) {
    md5_context         md5t;
    const static char   hex[] = "0123456789abcdef";
    unsigned char       md5h[16];
    int                 i;
    unsigned char data[33];
    int len;
    len = sprintf_s((char *)data,sizeof(data),"%s%s",pass,sesskey);
    md5_starts(&md5t);
    md5_update(&md5t, data, len);
    md5_finish(&md5t, md5h);

    for(i = 0; i < 16; i++) {
        *out++ = hex[md5h[i] >> 4];
        *out++ = hex[md5h[i] & 15];
    }
    *out = 0;
}
void Client::getResponse(int chrespnum, char *secretkey, char *out, int dstlen) {
    md5_context         md5t;
    const static char   hex[] = "0123456789abcdef";
    unsigned char       md5h[16];
    int                 i;
    unsigned char data[33];
    int len;
    len = sprintf_s((char *)data,sizeof(data),"%d%s",chrespnum,secretkey);
    md5_starts(&md5t);
    md5_update(&md5t, data, len);
    md5_finish(&md5t, md5h);

    for(i = 0; i < 16; i++) {
        *out++ = hex[md5h[i] >> 4];
        *out++ = hex[md5h[i] & 15];
    }
    *out = 0;
}
int Client::getProfileID() {
	return profileid;
}
int Client::getUserID() {
	return userid;
}
time_t Client::getLastPacket() {
	return lastPacket;
}
