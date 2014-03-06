#include <common/chc_endian.h>
#include "Client.h"
Client::Client(clientParams* params) {
	sd = params->sd;
	profileid = 0;
	userid = 0;
	quietflags = (GPEnum)0;
	game = NULL;
	ip = params->peer.sin_addr.s_addr;
	sentBuddies = false;
	sesskey = time(NULL) * rand();
	if(sesskey < 0) sesskey = -sesskey;
	memset(&challenge,0,sizeof(challenge));
	gen_random(challenge,10);
	sentAddRequests = true;
	memset(&statusstr,0,sizeof(statusstr));
	memset(&locstr,0,sizeof(locstr));
	memset(&email,0,sizeof(email));
	formatSend(sd,true,0,"\\lc\\1\\challenge\\%s\\id\\1",challenge);
	lastPacket = time(NULL);
	free((void *)params);
}
Client::~Client() {
	close(sd);
}
int Client::getSocket() {
	return sd;
}
void Client::mainLoop(fd_set *rset) {
	if(!FD_ISSET(sd,rset)) return;
	memset(&buff,0,sizeof(buff));
	len = recv(sd,buff,sizeof(buff),0);
	if(!do_db_check()) { 
		//send db error
		sendError(sd,false,"The database connection has been lost, please wait.",GP_DATABASE,1);
		return;
	}
	if(len<1) goto end;
	lastPacket = time(NULL);
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
		formatSend(sd,true,0,"%s",errtext);
	} else {
		formatSend(sd,true,0,"%s\\id\\%d",errtext,id);
	}
	free((void *)errtext);
	if(fatal) {
		deleteClient(this);
	}
	return;
}
void Client::loadBuddies() {
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[128];
	sprintf_s(query,len,"SELECT `targetid` FROM `Presence`.`buddies` WHERE `profileid` = '%d'",profileid);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	res = mysql_store_result(server.conn);
	buddies.clear();
	while ((row = mysql_fetch_row(res)) != NULL) {
		buddies.push_front(atoi(row[0]));
	}
  	mysql_free_result(res);
}
void Client::loadBlockedList() {
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[128];
	sprintf_s(query,len,"SELECT `blockedid` FROM `Presence`.`blocks` WHERE `profileid` = '%d'",profileid);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	res = mysql_store_result(server.conn);
	blocklist.clear();
	while ((row = mysql_fetch_row(res)) != NULL) {
		blocklist.push_front(atoi(row[0]));
	}
  	mysql_free_result(res);
}
void Client::handleLogin(char *buff, int len) {
	char uniquenick[GP_NICK_LEN],user[GP_EMAIL_LEN + GP_NICK_LEN + 2];
	char challenge[128];
	char response[33];
	char sdkrev[10];
	//normal clients send the userid and profileid to the server anyways but since its not required, lets just not bother with it
	memset(&uniquenick,0,sizeof(uniquenick));
	memset(&user,0,sizeof(user));
	find_param("uniquenick", buff, uniquenick, sizeof(uniquenick));
	find_param("user", buff, user, sizeof(user));
	if(find_param("sdkrevision",buff, sdkrev, sizeof(sdkrev))) {
		this->sdkrevision = atoi(sdkrev);
	}
	if(find_param("port",buff, sdkrev, sizeof(sdkrev))) {
		this->port = atoi(sdkrev);
	}
	if(!find_param("challenge", buff, challenge, sizeof(challenge))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	if(!find_param("response", buff, response, sizeof(response))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	if(uniquenick[0] != 0) {
		profileid = getProfileIDFromUniquenick(server.conn,uniquenick);
		if(profileid == 0) {
			sendError(sd,true,"The uniquenick provided is incorrect.",GP_LOGIN_BAD_UNIQUENICK,1);
			return;
		}
	} else if(user[0] != 0) {
		char *nick,*email;
		nick = (char *)&user;
		email = strchr(nick,'@');
		if(email == NULL) {
			sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
			return;
		}
		*email++=0;
		strncpy(this->email,email,strlen(email)%sizeof(this->email));
		profileid = getProfileIDFromNickEmail(server.conn, nick,email);
		*(email-1) = '@'; //for gs proof
		
	} else {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	char pass[GP_PASSWORD_LEN];
	char *proofuser = (char *)&user;
	if(uniquenick[0] != 0) proofuser = (char *)&uniquenick;
	getProfileIDPass(server.conn,profileid,(char *)&pass,sizeof(pass));
	if(strcmp((const char *)gs_login_proof((unsigned char *)&pass,(unsigned char *)proofuser,(unsigned char *)&this->challenge,(unsigned char *)&challenge),response) != 0) { //invalid password
		sendError(sd,true,"The password provided is incorrect.",GP_LOGIN_BAD_PASSWORD,1);
		return;
	}
	userid = getProfileUserID(server.conn,profileid);
	char lt[25]; //send the login ticket, not used yet and probably won't be by us but some clients expect it for other stuff, should probably find out what for, for maximum support
	memset(&lt,0,sizeof(lt));
	gen_random(lt,22);
	strcat(lt,"__");
	char suniquenick[GP_NICK_LEN];
	formatSend(sd,true,0,"\\lc\\2\\sesskey\\%d\\proof\\%s\\userid\\%d\\profileid\\%d\\uniquenick\\%s\\lt\\%s\\id\\1",sesskey,gs_login_proof((unsigned char *)&pass,(unsigned char *)proofuser,(unsigned char *)&challenge,(unsigned char *)&this->challenge),userid,profileid,uniquenick,lt);	
	loadBuddies();
	loadBlockedList();
	sendMessages();
	if(this->sdkrevision & GPI_NEW_LIST_RETRIEVAL_ON_LOGIN) {
		sendBuddies();
	}
}
void Client::sendAddRequests() {
	char query[256];
	sprintf_s(query,sizeof(query),"SELECT `profileid`,`syncrequested`,`reason` FROM `Presence`.`addrequest` WHERE `targetid` = %d",getProfileID());
	MYSQL_RES *res;
	MYSQL_ROW row;
	sentAddRequests = true;
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	res = mysql_store_result(server.conn);
	while ((row = mysql_fetch_row(res)) != NULL) {
		formatSend(sd,true,0,"\\bm\\%d\\f\\%d\\msg\\%s|signed|%s",GPI_BM_REQUEST,atoi(row[0]),row[2],row[1]);
//formatSend(c->sd,true,0,"\\bm\\%d\\f\\%d\\msg\\%s|signed|%s",GPI_BM_REQUEST,profileid,reason,signature);
	}
  	mysql_free_result(res);
	
}
void Client::sendBuddyInfo(int profileid) {
	Client *c = getProfile(profileid);
//\bm\100\f\104851729\msg\|s|0|ss|Offline\final
	char statstr[GP_STATUS_STRING_LEN + GP_STATUS_STRING_LEN + 128];
	if(c == NULL || c->hasBlocked(profileid)) {
		sprintf_s(statstr,sizeof(statstr),"|s|0|ss|Offline");
	} else {
		sprintf_s(statstr,sizeof(statstr),"|s|%d|ss|%s%s%s|ip|%d|p|%d|qm|%d",c->status,c->statusstr,c->locstr[0] != 0?"|ls|":"",c->locstr,reverse_endian32(c->ip),reverse_endian16(c->port),c->quietflags);
	}
	formatSend(sd,true,0,"\\bm\\%d\\f\\%d\\msg\\%s",GPI_BM_STATUS,profileid,statstr);
}
void Client::sendBuddies() {
	std::list<int>::iterator iterator;
	int pid = 0;
	this->sentBuddies = true;
	if(this->sdkrevision & GPI_NEW_LIST_RETRIEVAL_ON_LOGIN) {
		iterator=buddies.begin();
		char *bstr = (char *)calloc(256,1);
		int bstrlen = 256;
		char tstr[24];
		int numBuddies = buddies.size();
		int i = 0;
		while(iterator != buddies.end()) {
			pid = *iterator;
			sprintf_s(tstr,sizeof(tstr),"%d",pid);
			strcat(bstr,tstr);
			if(++i != numBuddies) {
				strcat(bstr,",");
			}
			if(strlen(bstr) > (bstrlen-128)) {
				bstrlen += 256;
				bstr = (char *)realloc(bstr,bstrlen);
			}
			iterator++;
		}
		formatSend(sd,true,0,"\\bdy\\%d\\list\\%s",numBuddies,bstr);
		free((void *)bstr);
//		return; //we want to send the buddy info too 
	}
	iterator = buddies.begin();
	while(iterator != buddies.end()) {
		pid = *iterator;
		sendBuddyInfo(pid);
		iterator++;
	}
}
void Client::handleNewUser(char *buff, int len) {
	//TODO: register a user if not registered
	char uniquenick[GP_NICK_LEN+1],nick[GP_NICK_LEN+1],email[GP_EMAIL_LEN+1],pass[GP_PASSWORD_LEN+1],cdkey[GP_CDKEY_LEN+1];
	if(!find_param("email", buff, email, sizeof(email))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	if(!find_param("password", buff, pass, sizeof(pass))) {
		//todo: add passenc
		if(!find_param("passenc",buff,pass,sizeof(pass))) {
			sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		       return;	
		} else {
			char *dpass;
			int passlen = strlen(pass);
			dpass = (char *)base64_decode((uint8_t *)pass, &passlen);
        		passlen = gspassenc((uint8_t *)dpass);
			strcpy(pass,dpass);
			free(dpass);
		}
	}
	if(!find_param("cdkey",buff,cdkey,sizeof(cdkey))) {
		if(find_param("cdkeyenc",buff,cdkey,sizeof(cdkey))) {
			char *dkey;
			int keylen = strlen(cdkey);
			dkey = (char *)base64_decode((uint8_t *)cdkey, &keylen);
        		keylen = gspassenc((uint8_t *)dkey);
			strcpy(cdkey,dkey);
			free(dkey);

		}
	}
	if(!find_param("nick", buff, nick, sizeof(nick))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	char gamename[64]; //theres no true max gamename len but this is bigger than them all
	if(find_param("gamename",buff,gamename,sizeof(gamename))) {
		game = server.options->gameInfoNameProc((char *)gamename);
	}
	find_param("uniquenick", buff, uniquenick, sizeof(uniquenick));
	int userid = getUserIDFromEmail(server.conn,email);
	if(userid == 0) {
		userid = registerUser(server.conn,email,pass);
		profileid = makeNewProfile(server.conn,nick,userid);
		formatSend(sd,true,0,"\\nur\\\\userid\\%d\\profileid\\%d",userid,profileid);
		return;
	}
	if(!tryPassword(server.conn,userid,pass)) {
		sendError(sd,true,"The password provided is incorrect.",GP_NEWUSER_BAD_PASSWORD,1);
		return;
	}
	int profileid = 0;
	if(uniquenick[0] != 0) {
		profileid = getProfileIDFromUniquenick(server.conn, uniquenick);
		if(profileid != 0) {
		//A profile with that nick already exists.
			sendError(sd,true,"A profile with this uniquenick already exists.",GP_NEWUSER_UNIQUENICK_INUSE,1);
			return;
		}
		if(!nameValid(uniquenick,false)) {
			sendError(sd,true,"The requested Uniquenick contains invalid characters.",GP_NEWUSER_UNIQUENICK_INVALID,1);
			return;
		}
	} else { 
		profileid = getProfileIDFromNickEmail(server.conn,nick,email);
		if(profileid != 0) {
			char errmsg[256];
			sprintf_s(errmsg,sizeof(errmsg),"A profile with that nick already exists.\\pid\\%d",profileid);
			sendError(sd,true,errmsg,GP_NEWUSER_BAD_NICK,1);
			return;
		} else {
			if(!nameValid(nick,false)) {
				sendError(sd,true,"This name contains invalid characters.",GP_NEWUSER_BAD_NICK,1);
				return;
			}
			profileid = makeNewProfile(server.conn,nick,userid);
			if(cdkey[0] != 0) {
				addAuthCdKey(server.conn,profileid, cdkey,game);
			}
			formatSend(sd,true,0,"\\nur\\\\userid\\%d\\profileid\\%d",userid,profileid);
			return;
		}
	}
	
}
void Client::handleStatus(char *buff, int len) {
	char type[GP_STATUS_STRING_LEN];
	if(find_param("status", buff, type, sizeof(type))) {
		this->status = (GPEnum)atoi(type);
	}
	find_param("statstring", buff, statusstr, sizeof(statusstr));
	find_param("locstring", buff, locstr, sizeof(locstr));
	if(!this->sentBuddies) {
		sendBuddies();
	}
	if(!this->sentAddRequests) {
		sendAddRequests();
	}
	sendStatusUpdateToBuddies(this);
}
void Client::handleGetProfile(char *buff, int len) {
	int lenx = 1024;
	char *sbuff = (char *)malloc(lenx);
	memset(sbuff,0,lenx);
	char tbuff[256];
	if(!this->sentBuddies) {
		sendBuddies();
	}
	int id = find_paramint("id",buff);
	int profileid = find_paramint("profileid",buff); 
	#define addString(b,l,s,n) if(s != NULL) { \
					if(strlen(b) + strlen(s) > (l-128)) { \
						lenx+=128;\
						b = (char *)realloc(b,lenx);\
					} \
					strcat(b,n); \
					strcat(b,s);\
				  }
	#define addStringNull(b,l,s,n)  { \
					if(s != NULL && strlen(b) + strlen(s) > (l-128)) { \
						lenx+=128;\
						b = (char *)realloc(b,lenx);\
					} \
					strcat(b,n); \
					if(s != NULL) \
					strcat(b,s);\
				  }
	#define addStringStack(b,l,s,n) if(s[0] != 0) { \
					if(strlen(b) + strlen(s) > (l-128)) { \
						lenx+=128;\
						b = (char *)realloc(b,lenx);\
					} \
					strcat(b,n); \
					strcat(b,s);\
				  }
	#define addStringStackNull(b,l,s,n) { \
					if(s[0] != 0 && strlen(b) + strlen(s) > (l-128)) { \
						lenx+=128;\
						b = (char *)realloc(b,lenx);\
					} \
					strcat(b,n); \
					if(s[0] != 0) \
					strcat(b,s);\
				  }
	#define addFloat(b,l,f,n) sprintf_s(tbuff,sizeof(tbuff),"%s%f",n,f); \
				  strcat(b,tbuff);
	#define addInt(b,l,f,n) sprintf_s(tbuff,sizeof(tbuff),"%s%d",n,f); \
				  strcat(b,tbuff);
	GPIInfoCache info;
	if(!getProfileInfo(server.conn,profileid,((GPIInfoCache *)&info))) {
	} else {
		uint32_t publicmask = getPublicMask(server.conn,info.userid);
		char signature[33];
		strcat(sbuff,"\\pi\\");
		addInt(sbuff,lenx,info.profileid,"\\profileid\\")
		addString(sbuff,lenx,info.nick,"\\nick\\")
		addInt(sbuff,lenx,info.userid,"\\userid\\")	
		if(publicmask & GP_MASK_EMAIL || info.profileid == getProfileID()) {
			addString(sbuff,lenx,info.email,"\\email\\")
		}
		addString(sbuff,lenx,signature,"\\sig\\")
		addString(sbuff,lenx,info.uniquenick,"\\uniquenick\\")
/*		addString(sbuff,lenx,info.firstname,"\\firstname\\")
		addString(sbuff,lenx,info.lastname,"\\lastname\\")
		if(publicmask & GP_MASK_COUNTRYCODE || info.profileid == getProfileID()) {
			addStringStack(sbuff,lenx,info.countrycode,"\\countrycode\\")
		}
		addStringNull(sbuff,lenx,info.aimname,"\\aim\\")
		addInt(sbuff,lenx,info.publicmask,"\\pmask\\")
		addInt(sbuff,lenx,info.pic,"\\pic\\")
		addInt(sbuff,lenx,info.occupationid,"\\ooc\\")
		addInt(sbuff,lenx,info.industryid,"\\ind\\")
		addInt(sbuff,lenx,info.incomeid,"\\inc\\")
		addInt(sbuff,lenx,info.marriedid,"\\mar\\")
		addInt(sbuff,lenx,info.childcount,"\\chc\\")
		addInt(sbuff,lenx,info.interests1,"\\i1\\")
		addInt(sbuff,lenx,info.ownership1,"\\o1\\")
		addInt(sbuff,lenx,info.conntypeid,"\\conn\\")
		if(publicmask & GP_MASK_SEX || info.profileid == getProfileID()) {
			addInt(sbuff,lenx,info.sex,"\\sex\\")
		}
		if(publicmask & GP_MASK_ZIPCODE || info.profileid == getProfileID()) {
			addStringStackNull(sbuff,lenx,info.zipcode,"\\zipcode\\")
		}
		if(publicmask & GP_MASK_HOMEPAGE || info.profileid == getProfileID()) {
			addString(sbuff,lenx,info.homepage,"\\homepage\\")
		}
		if(publicmask & GP_MASK_BIRTHDAY || info.profileid == getProfileID()) {
			uint32_t birthday = 0;
			birthday |= (info.birthday << 24);
			birthday |= (info.birthmonth << 16);
			birthday |= info.birthyear;
			addInt(sbuff,lenx,birthday,"\\birthday\\")
		}
*/		addFloat(sbuff,lenx,info.longitude,"\\lon\\")
		addFloat(sbuff,lenx,info.latitude,"\\lat\\")
//		addStringStackNull(sbuff,lenx,info.place,"\\loc\\")
		gs_login_proof_md5((unsigned char *)&sbuff,strlen(sbuff),(unsigned char *)&signature);
		#define infoFree(x) if(x != NULL) free((void *)x);
		formatSend(sd,true,0,"%s\\id\\%d",sbuff,id);
		infoFree(info.nick)
		infoFree(info.uniquenick)
		infoFree(info.email)
		infoFree(info.firstname)
		infoFree(info.lastname)
		infoFree(info.homepage)
		infoFree(info.aimname)
		#undef infoFree
		#undef addString
		#undef addStringStack
		#undef addInt
		#undef addFloat
		#undef addStringNull
		#undef addStringStackNull
	}
	free((void *)sbuff);
}
void Client::handleBM(char *buff, int len) {
//\bm\1\sesskey\3483058\t\157928340\msg\FzJew fzmanlove\final
	int type = find_paramint(1,buff);
	int to = find_paramint("t",buff);
	char buffer[2048];
	char query[2048 + 512];
	Client *target = getProfile(to);
	if(target != NULL) {
		if(target->hasBlocked(this)) {
			return;
		}
	}
	switch(type) {
		case GPI_BM_PING:
		case GPI_BM_PONG:
		case GPI_BM_UTM:
		case GPI_BM_MESSAGE:
			if(find_param("msg",buff,buffer,sizeof(buffer))) {
				if(target != NULL) {
					if(target->hasBuddy(this) || hasBuddy(target)) {
						formatSend(target->sd,true,0,"\\bm\\%d\\f\\%d\\msg\\%s",type,profileid,buffer);
					} else {
						sendError(sd,false,"Trying to message someone who is not your buddy.",GP_BM_NOT_BUDDY,1);
						return;
					}
				} else { //save to db so they can recieve it when they reconnect
					if(type != GPI_BM_MESSAGE) {
						sendError(sd,false,"Buddy is offline",GP_BM_BUDDY_OFFLINE,1);
					} else {
						saveMessage(to,buffer);
					}
					return;
				}
			}
		break;
		default:
		printf("Unknown BM type: %02X\n",type);
		sendError(sd,true,"Invalid Type in BM command",GP_PARSE,1);
		return;
		break;
	}
}
void Client::sendMessages() {
	char query[256];
	MYSQL_RES *res;
	MYSQL_ROW row;
	sprintf_s(query,sizeof(query),"SELECT `message`,`from`,Unix_Timestamp(`date`) FROM `Presence`.`messages` WHERE `to` = %d",getProfileID());
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
	res = mysql_store_result(server.conn);
	while ((row = mysql_fetch_row(res)) != NULL) {
		formatSend(sd,true,0,"\\bm\\%d\\f\\%d\\date\\%d\\msg\\%s",GPI_BM_MESSAGE,atoi(row[1]),atoi(row[2]),row[0]);
	}
  	mysql_free_result(res);
	sprintf_s(query,sizeof(query),"DELETE FROM `Presence`.`messages` WHERE `to` = %d",getProfileID());
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
}
void Client::saveMessage(int profileid,char *msg) {
	if(getProfile(profileid) != NULL) return;
	int len = 512 + ((strlen(msg)*2) + 1);
	char *smsg = (char *)malloc((strlen(msg) * 2) + 1);
	char *query = (char *)malloc(len);
	mysql_real_escape_string(server.conn,smsg,msg,strlen(msg));
	sprintf_s(query,len,"INSERT INTO `Presence`.`messages` SET `to` = %d, `from` = %d, `message` = \"%s\"",profileid,getProfileID(),smsg);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
	}
	free((void *)query);
	free((void *)smsg);
}
void Client::handleAddBuddy(char *buff, int len) {
	char reason[GP_REASON_LEN+1];
	// Signature in the raw consists of from profile id, new profile id, 
	char hashProfileInfo[10 + 10 + GP_PASSWORD_LEN];
	char signature[33];
	int newprofileid;
	char query[512];
	//\authadd\\sesskey\3466925\fromprofileid\118498678\sig\574887b8a270a052a79527d421753d1b\final
	//\addbuddy\\sesskey\3483058\newprofileid\157928340\reason\\final
	memset(&reason,0,sizeof(reason));
	find_param("reason",buff,reason,sizeof(reason));
	newprofileid = find_paramint("newprofileid",buff);
	if(hasBuddy(newprofileid) || getProfileUserID(server.conn, newprofileid) == 0) {
		return;
	}
	find_param("syncrequested",buff,hashProfileInfo,sizeof(hashProfileInfo));
	char md5buff[64];
	int lenx = sprintf_s(md5buff,sizeof(md5buff),"%d%d",profileid,newprofileid);
	gs_login_proof_md5((unsigned char *)&md5buff,lenx,(unsigned char *)&signature);
	Client *c;
	mysql_real_escape_string(server.conn,hashProfileInfo,hashProfileInfo,strlen(hashProfileInfo));
	mysql_real_escape_string(server.conn,reason,reason,strlen(reason));
	sprintf_s(query,sizeof(query),"INSERT INTO `Presence`.`addrequest` (`profileid`,`targetid`,`syncrequested`,`reason`) VALUES (%d,%d,\"%s\",\"%s\")",profileid,newprofileid,hashProfileInfo,reason);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	if((c = getProfile(newprofileid)) != NULL) { 
		formatSend(c->sd,true,0,"\\bm\\%d\\f\\%d\\msg\\%s|signed|%s",GPI_BM_REQUEST,profileid,reason,signature);
	}

}
void Client::handleDelBuddy(char *buff, int len) {
//deleteBuddy(
	int pid = find_paramint("delprofileid",buff);
	if(!hasBuddy(pid)) {
		sendError(sd,false,"You do not have this buddy.",GP_DELBUDDY_NOT_BUDDY,1);
		return;
	}
	char query[256];
	sprintf_s(query,sizeof(query),"DELETE FROM `Presence`.`buddies` WHERE `profileid` = %d AND targetid = %d",profileid,pid);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	deleteBuddy(pid);
	/* removed because this is for deleteing your buddy - not a buddy who has you from their list(todo: find that? probably exists)
	Client *c;
	if((c = getProfile(pid)) != NULL) {
		//pretend they went offline unless we can find a gamespy server msg saying your friend deleted you to remove them from your list too
		formatSend(c->sd,true,0,"\\bm\\%d\\f\\%d\\msg\\|s|0|ss|Offline",GPI_BM_STATUS,profileid);
		return;
	}
	*/
}
void Client::handleAuthAdd(char *buff, int len) {
	//TODO: check if they are actually trying to add you
	MYSQL_RES *res;
	MYSQL_ROW row;
	int fromprofileid;
	char query[256];
	fromprofileid = find_paramint("fromprofileid",buff);
	sprintf_s(query,sizeof(query),"SELECT 1 FROM `Presence`.`addrequest` WHERE `profileid` = '%d' AND `targetid` = '%d'",fromprofileid,profileid);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	Client *c = getProfile(fromprofileid);
	res = mysql_store_result(server.conn);
	int num = mysql_num_rows(res);
	//\bm\1\f\157928340\msg\I have authorized your request to add me to your list\final
	if(c == NULL) goto end;
	if(num > 0) {
		formatSend(c->sd,true,0,"\\bm\\%d\\f\\%d",GPI_BM_AUTH,fromprofileid);
		c->sendBuddyInfo(this->getProfileID());
	}
	sprintf_s(query,sizeof(query),"INSERT INTO `Presence`.`buddies` SET `profileid` = %d, `targetid` = %d",fromprofileid,profileid);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
	c->buddies.push_back(profileid);
end:
  	mysql_free_result(res);
	
}
void Client::handleRevoke(char *buff, int len) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[256];
	int pid = find_paramint("profileid",buff);
	sprintf_s(query,sizeof(query),"SELECT 1 FROM `Presence`.`buddies` WHERE `profileid` = '%d' AND `targetid` = '%d'",pid,profileid);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	if(mysql_num_rows(res) < 1) {
		sendError(sd,false,"This buddy does not have you on his list.",GP_REVOKE_NOT_BUDDY,1);
		mysql_free_result(res);
		return;
	}
	mysql_free_result(res);
	res = mysql_store_result(server.conn);
	Client *c;
	if((c = getProfile(pid)) != NULL) {
		formatSend(c->sd,true,0,"\\bm\\%d\\f\\%d\\date\\%d",GPI_BM_REVOKE,profileid,time(NULL));
	}
	sprintf_s(query,sizeof(query),"DELETE FROM `Presence`.`buddies` WHERE `profileid` = %d AND targetid = %d",pid,profileid);
	if (mysql_query(server.conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(server.conn));
		return;
	}
	return;
}
void Client::handleUpdateProfile(char *buff, int len) {
	GPIInfoCache info;
	memset(&info,0,sizeof(GPIInfoCache));
	char tbuff[256];
	getProfileInfo(server.conn,getProfileID(),((GPIInfoCache *)&info));
	#define setParamInt(x,y) if(find_param(x,buff,tbuff,sizeof(tbuff))) { \
					y = atoi(tbuff);\
				}
	#define setParam(x,y) if(find_param(x,buff,tbuff,sizeof(tbuff))) { \
				if(y != NULL) free((void *)y); \
				y = (char *)malloc(strlen(tbuff)+1);\
				if(y != NULL) strcpy(y,tbuff); \
	}
	#define setParamStack(x,y) if(find_param(x,buff,tbuff,sizeof(tbuff))) { \
				strncpy(y,tbuff,strlen(tbuff)%sizeof(y)); \
	}
	#define setParamFloat(x,y) if(find_param(x,buff,tbuff,sizeof(tbuff))) { \
					y = atof(tbuff);\
				}
	setParam("nick",info.nick)
	setParam("uniquenick",info.uniquenick)
	setParam("firstname",info.firstname)
	setParam("lastname",info.lastname)
	setParamStack("loc",info.place)
/*	setParam("aimname",info.aimname)
	setParam("homepage",info.homepage)
	setParamStack("countrycode",info.countrycode)
	setParamStack("zipcode",info.zipcode)
	setParamInt("pic",info.pic)
	setParamInt("sex",info.sex)
	setParamInt("ind",info.industryid)
	setParamInt("inc",info.incomeid)
	setParamInt("mar",info.marriedid)
	setParamInt("chc",info.childcount)
	setParamInt("i1",info.interests1)
	setParamInt("o1",info.ownership1)
	setParamInt("conn",info.conntypeid)
	setParamInt("publicmask",info.publicmask)
	if(find_param("birthday",buff,tbuff,sizeof(tbuff))) { 
		uint32_t birthday = atoi(tbuff);
		setParamInt("birthday",birthday)
		info.birthday = ((birthday >> 24) & 0xFF);
		info.birthmonth = ((birthday >> 16) & 0xFF);
		info.birthyear = (birthday & 0xFFFF);
	}
	setParamInt("icquin",info.icquin)
*/	setParamFloat("lat",info.latitude)
	setParamFloat("lon",info.longitude)
	updateUserProfile(server.conn,(GPIInfoCache *)&info);
	#define infoFree(x) if(x != NULL) free((void *)x);
	infoFree(info.nick)
	infoFree(info.uniquenick)
	infoFree(info.email)
	infoFree(info.firstname)
	infoFree(info.lastname)
//	infoFree(info.homepage)
//	infoFree(info.aimname)
	#undef infoFree
	#undef setParamInt
	#undef setParam
	#undef setParamStack
	#undef setParamFloat
}
void Client::handleNewProfile(char *buff, int len) {
//\newprofile\\sesskey\949406328\nick\testuseraaaaaaaaa\id\2\final\\npr\\profileid\365566860\\id\2\final
	char nick[(GP_NICK_LEN*2) + 1];
	char query[256];
	int profileid = 0;
	bool replace = false;
	memset(&nick,0,sizeof(nick));
	replace = find_paramint("replace",buff)==1?true:false;
	if(!find_param("nick",buff,nick,sizeof(nick))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	if(replace) {
		profileid = getProfileIDFromNickEmail(server.conn, nick, (char *)&email);
		if(profileid != 0) {
			return;
		}
		mysql_real_escape_string(server.conn,nick,nick,strlen(nick));
		sprintf_s(query,sizeof(query),"UPDATE `GameTracker`.`profiles` SET `nick` = '%s' WHERE `profileid` = %d",nick,getProfileID());
		if(mysql_query(server.conn,query)) {
			fprintf(stderr,"%s\n",mysql_error(server.conn));
			return;
		}
		profileid = getProfileID();
	} else {
		profileid = makeNewProfile(server.conn, nick, getUserID());
	}
	formatSend(sd,true,0,"\\npr\\\\profileid\\%d\\\\id\\2\\",profileid);
	if(replace) {
		deleteClient(this);
	}
}
void Client::handleDelProfile(char *buff, int len) {
	char query[256];
	int id = find_paramint("id",buff);
	sprintf_s(query,sizeof(query),"UPDATE `GameTracker`.`profiles` SET `deleted` = 1 WHERE `profileid` = %d",getProfileID());
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
	formatSend(sd,true,0,"\\dpr\\\\id\\%d",id);
	deleteClient(this);
}
void Client::handleAddBlock(char *buff, int len) {
	int pid = find_paramint("profileid",buff);
	char query[256];
	if(hasBlocked(pid)) {
		return;
	}
	sprintf_s(query,sizeof(query),"INSERT INTO `Presence`.`blocks` SET `profileid` = %d, `blockedid` = %d",getProfileID(),pid);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
	blocklist.push_back(pid);
}
void Client::handleRemoveBlock(char *buff, int len) {
	int pid = find_paramint("profileid",buff);
	char query[256];
	if(!hasBlocked(pid)) {
		return;
	}
	sprintf_s(query,sizeof(query),"DELETE FROM `Presence`.`blocks` WHERE `profileid` = %d AND `blockedid` = %d",getProfileID(),pid);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
	deleteBlock(pid);

}
void Client::handleInviteTo(char *buff, int len) {
	char products[256];
	int product = 0;
	char *pch;
	if(!find_param("products",buff,products,sizeof(products))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,1);
		return;
	}
	inviteableProducts.clear();
	if(strchr(products,'\\') != NULL) {
	pch = strtok(",",products);
		while(pch != NULL) {
			product = atoi(pch);
			inviteableProducts.push_front(product);
			pch = strtok(",", NULL);
		}
	} else {
		product = atoi(products);
		inviteableProducts.push_front(product);
	}
}
void Client::handlePlayerInvite(char *buff, int len) {
	int product = find_paramint("productid",buff);
	int pid = find_paramint("profileid",buff);
	Client *c = getProfile(pid);
	if(c == NULL) return;
	if(c->productInviteable(product) && !c->hasBlocked(this)) {
		formatSend(c->sd,true,0,"\\bm\\%d\\f\\%d\\msg\\|p|%d",GPI_BM_INVITE,getProfileID(),product);
	}
}
void Client::handleRegisterNick(char *buff, int len) {
	int id = find_paramint("id",buff);
	char uniquenick[(GP_UNIQUENICK_LEN*2)+1];
	char query[256];
	if(!find_param("uniquenick",buff,uniquenick,sizeof(uniquenick))) {
		sendError(sd,true,"There was an error parsing a request.",GP_PARSE,id);
		return;
	}
	if(getProfileIDFromUniquenick(server.conn, (char *)&uniquenick)) {
		sendError(sd,false,"The uniquenick is already taken.",GP_REGISTERUNIQUENICK_TAKEN,id);
		return;
	}
	mysql_real_escape_string(server.conn,uniquenick,uniquenick,strlen(uniquenick));
	sprintf_s(query,sizeof(query),"UPDATE `GameTracker`.`profiles` SET `uniquenick` = '%s' WHERE `profileid` = %d",uniquenick,getProfileID());
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
	formatSend(sd,true,0,"\\rn\\%s\\id\\%d",uniquenick,id);
}
void Client::handleUpdateUI(char *buff, int len) {
	char email[(GP_EMAIL_LEN*2)+1];
	char pass[(GP_PASSWORD_LEN*2)+1];
	int id = find_paramint("id",buff);
	int uid;
	char query[256];
	bool emailChanged = true,passChanged = false;
	if(!find_param("email",buff,(char *)&email,GP_EMAIL_LEN)) {
		//sendError(sd,true,"There was an error parsing a request.",GP_PARSE,id);
		if(uid = (getUserIDFromEmail(server.conn, email) != getUserID())) {
			if(uid != 0) {
				sendError(sd,false,"A user with the email address provided already exists.",GP_UPDATEUI_BAD_EMAIL,id);
				return;
			}
			mysql_real_escape_string(server.conn,email,email,strlen(email));
			sprintf_s(query,sizeof(query),"UPDATE `GameTracker`.`users` SET `email` = %s, `emailverified` = 0 WHERE `userid` = %d",email,getUserID());			
			if(mysql_query(server.conn,query)) {
				fprintf(stderr,"%s\n",mysql_error(server.conn));
				return;
			}
		}
	}
    if(!find_param("pass", buff, pass, sizeof(pass))) {
	if(!find_param("passwordenc",buff,pass,sizeof(pass))) {
	       getUserIDPass(server.conn, getUserID(), (char *)&pass, GP_PASSWORD_LEN);
	} else {
		char *dpass;
		int passlen = strlen(pass)%GP_PASSWORD_LEN;
		dpass = (char *)base64_decode((uint8_t *)pass, &passlen);
        	passlen = gspassenc((uint8_t *)dpass);
		strcpy(pass,dpass);
		free(dpass);
	}
   }
   mysql_real_escape_string(server.conn,pass,pass,strlen(pass));
   sprintf_s(query,sizeof(query),"UPDATE `GameTracker`.`users` SET `password` = '%s' WHERE `userid` = %d",pass,getUserID());
   if(mysql_query(server.conn,query)) {
	fprintf(stderr,"%s\n",mysql_error(server.conn));
	return;
   }
	
}
void Client::parseIncoming() {
	char *p = (char *)&buff;	
	char *x;
	while(true) {
		x = p;
		p = strstr(p,"\\final\\");
		if(p == NULL) { break; }
		*p = 0;
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
	if(strcmp(type,"login") == 0) {
		handleLogin(buff,len);
		return;
	} 
	else if(strcmp(type,"newuser") == 0) {
		handleNewUser(buff,len);
		return;
	} else if(strcmp(type,"ka") == 0) { //keep alive
		formatSend(sd,true,0,"\\ka\\");
		return;
	} else if(strcmp(type,"inviteto") == 0) {
		handleInviteTo(buff,len);
		return;
	}
	if(userid == 0) {
		sendError(sd,true,"This request cannot be processed because you are not logged in.",GP_NOT_LOGGED_IN,-1);
		deleteClient(this);
		return;
	}
	if(strcmp(type,"status") == 0) {
		handleStatus(buff,len);
	}else if(strcmp(type,"getprofile") == 0) {
		handleGetProfile(buff,len);
	} else if(strcmp(type,"addbuddy") == 0) {
		handleAddBuddy(buff,len);
	} else if(strcmp(type,"authadd") == 0) {
		handleAuthAdd(buff,len);
	} else if(strcmp(type,"addblock") == 0) {
		handleAddBlock(buff,len);
	} else if(strcmp(type,"removeblock") == 0) {
		handleRemoveBlock(buff,len);
	} else if(strcmp(type,"delbuddy") == 0) {
		handleDelBuddy(buff,len);
	} else if(strcmp(type,"revoke") == 0) {
		handleRevoke(buff,len);
	} else if(strcmp(type,"bm") == 0) {
		handleBM(buff,len);
	} else if(strcmp(type,"updatepro") == 0) {
		handleUpdateProfile(buff,len);
	} else if(strcmp(type,"updateui") == 0) {
		handleUpdateUI(buff,len);
	} else if(strcmp(type,"registernick") == 0) {
		handleRegisterNick(buff,len);
	} else if(strcmp(type,"newprofile") == 0) {
		handleNewProfile(buff,len);
	} else if(strcmp(type,"delprofile") == 0) {
		handleDelProfile(buff,len);
	} else if(strcmp(type,"pinvite") == 0) {
		handlePlayerInvite(buff,len);			
	} else if(strcmp(type,"logout") == 0) {
		deleteClient(this);
	} else {
		printf("Unknown Command: %s\n",buff);
		return;
	}
}
bool Client::hasBuddy(Client *c) {
	return hasBuddy(c->getProfileID());
}
bool Client::hasBuddy(int upid) {
	std::list<int>::iterator iterator;
	iterator=buddies.begin();
	int pid = 0;
	while(iterator != buddies.end()) {
		pid = *iterator;
		if(upid == pid) return true;
		iterator++;
	}
	return false;
}
bool Client::hasBlocked(Client *c) {
	return hasBlocked(c->getProfileID());
}
bool Client::hasBlocked(int upid) {
	std::list<int>::iterator iterator;
	iterator=blocklist.begin();
	int pid = 0;
	while(iterator != blocklist.end()) {
		pid = *iterator;
		if(upid == pid) return true;
		iterator++;
	}
	return false;
}
void Client::sendBuddyStatus(Client *c) {
	sendBuddyInfo(c->getProfileID());
}
int Client::getProfileID() {
	return profileid;
}
int Client::getUserID() {
	return userid;
}
void Client::deleteBuddy(int profileid) {
	std::list<int>::iterator iterator;
	iterator = buddies.begin();
	int pid = 0;
	while(iterator != buddies.end()) {
		pid = *iterator;
		if(pid == profileid) {
			buddies.erase(iterator);
			return;
		}
		iterator++;
	}
}
void Client::deleteBlock(int profileid) {
	std::list<int>::iterator iterator;
	iterator = blocklist.begin();
	int pid = 0;
	while(iterator != blocklist.end()) {
		pid = *iterator;
		if(pid == profileid) {
			blocklist.erase(iterator);
			return;
		}
		iterator++;
	}
}
bool Client::productInviteable(int productid) {
	std::list<int>::iterator iterator;
	iterator = inviteableProducts.begin();
	int pid = 0;
	while(iterator != inviteableProducts.end()) {
		pid = *iterator;
		if(pid == profileid) {
			return true;
		}
		iterator++;
	}
	return false;
}
time_t Client::getLastPacket() {
	return lastPacket;
}
