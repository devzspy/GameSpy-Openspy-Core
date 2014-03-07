#include "lookup.h"
void checkEmailValid(int sd,char *buff) {
char data[GP_EMAIL_LEN];
char query[256];
    if(!find_param("email", buff, data,sizeof(data))) {
       sendError(sd,"Error recieving request");
       return;	
   }
   mysql_real_escape_string(conn,data,data,strlen(data));
   sprintf(query,"SELECT 1 FROM `users` WHERE `email` = '%s'",data);
   if (mysql_query(conn, query)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
	return;
   }
   res = mysql_store_result(conn);
   int i = mysql_num_rows(res);
   mysql_free_result(res);
   formatSend(sd,true,0,"\\vr\\%d",i);
}
void sendNicks(int sd, char *buff) {
    char email[GP_EMAIL_LEN];
    char pass[GP_PASSWORD_LEN];
    char query[1024];
    char gamename[32];
    bool sendUnique = false;
    if(!find_param("email", buff, email, sizeof(email))) {
       sendError(sd,"Error recieving request");
       return;	
    }
    if(find_param("gamename", buff, gamename, sizeof(gamename))) {
       sendUnique = true;//just assume you want it sent
    }
    if(!find_param("pass", buff, pass, sizeof(pass))) {
	if(!find_param("passenc",buff,pass,sizeof(pass))) {
	       sendError(sd,"Error recieving request");
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
   mysql_real_escape_string(conn,email,email,strlen(email));
   mysql_real_escape_string(conn,pass,pass,strlen(pass));
   sprintf_s(query,sizeof(query),"SELECT  `nick`,`uniquenick` FROM  `GameTracker`.`users` INNER JOIN  `GameTracker`.`profiles` ON  `GameTracker`.`profiles`.`userid` = `GameTracker`.`users`.`userid` WHERE  `GameTracker`.`users`.`email` =  '%s' AND  `password` =  '%s'",email,pass);
   if (mysql_query(conn, query)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
	return;
   }
	res = mysql_store_result(conn);
	char *sendmsg;
	char namestr[256];
	int num_rows = mysql_num_rows(res);
	if(num_rows == 0) {
		formatSend(sd,true,0,"\\nr\\\\ndone\\");
		return;
	}
	int len = 1024;
	sendmsg = (char *)malloc(len);
	memset((void *)sendmsg,0,len);
	sprintf_s(sendmsg,len,"\\nr\\%d",num_rows);
	while ((row = mysql_fetch_row(res)) != NULL) {
		sprintf_s(namestr,sizeof(namestr),"\\nick\\%s",row[0]);
		strncat(sendmsg,namestr,strlen(namestr)%len);
		if(sendUnique) {
			sprintf_s(namestr,sizeof(namestr),"\\uniquenick\\%s",row[1]);
			strncat(sendmsg,namestr,strlen(namestr)%len);
		}
		if(strlen(sendmsg)>(len/2)) {
			len *= 2;
			sendmsg = (char *)realloc(sendmsg,len);			
		}
		
        }
	strcat(sendmsg,"\\ndone\\");
	formatSend(sd,true,0,"%s",sendmsg);
        mysql_free_result(res);
	free((void *)sendmsg);
}
void checkNick(int sd, char *buff) {
	//TODO: add uniquenick support
	char nick[GP_NICK_LEN],email[GP_EMAIL_LEN],pass[GP_PASSWORD_LEN];
	char sendbuff[512];
	int userid,profileid;
	if(!find_param("nick", buff, nick, sizeof(nick))) {
	       sendError(sd,"Error recieving request");
	       return;	
	}
	mysql_real_escape_string(conn,nick,nick,strlen(nick));
	if(!find_param("email", buff, email, sizeof(email))) {
	       sendError(sd,"Error recieving request");
	       return;	
	}
	mysql_real_escape_string(conn,email,email,strlen(email));
	if(!find_param("pass", buff, pass, sizeof(pass))) {
		if(!find_param("passenc",buff,pass,sizeof(pass))) {
		       sendError(sd,"Error recieving request");
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
	mysql_real_escape_string(conn,pass,pass,strlen(pass));
	userid = getUserIDFromEmail(conn,email);
	if(userid == 0) {
		formatSend(sd,true,0,"\\cur\\%d",GP_CHECK_BAD_EMAIL);
		return;
	}
	if(!tryPassword(conn,userid,pass)) {
		formatSend(sd,true,0,"\\cur\\%d",GP_CHECK_BAD_PASSWORD);
		return;
	}
	profileid = getProfileIDFromNickEmail(conn, nick, email);
	if(profileid == 0) {
		formatSend(sd,true,0,"\\cur\\%d",GP_CHECK_BAD_NICK);
		return;
	}
	formatSend(sd,true,0,"\\cur\\0\\pid\\%d",profileid);
	return;
}
void newUser(int sd, char *buff) {
	//TODO: add uniquenick support
	char sendbuff[512];
	char nick[GP_NICK_LEN],email[GP_EMAIL_LEN],pass[GP_PASSWORD_LEN],uniquenick[GP_NICK_LEN];
	int userid,profileid;
	find_param("uniquenick",buff,uniquenick,sizeof(uniquenick));
	if(!find_param("nick", buff, nick, sizeof(nick))) {
	       sendError(sd,"Error recieving request");
	       return;	
	}
	mysql_real_escape_string(conn,nick,nick,strlen(nick));
	if(!find_param("email", buff, email, sizeof(email))) {
	       sendError(sd,"Error recieving request");
	       return;	
	}
	mysql_real_escape_string(conn,email,email,strlen(email));
	if(!find_param("pass", buff, pass, sizeof(pass))) {
		if(!find_param("passenc",buff,pass,sizeof(pass))) {
		       sendError(sd,"Error recieving request");
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
	mysql_real_escape_string(conn,pass,pass,strlen(pass));
	userid = getUserIDFromEmail(conn,email);
	if(userid == 0 || !tryPassword(conn,userid,pass)) {
		formatSend(sd,true,0,"\\nur\\%d",GP_NEWUSER_BAD_PASSWORD);
		return;
	}
	profileid = getProfileIDFromNickEmail(conn, nick, email);
	if(profileid != 0) {
		formatSend(sd,true,0,"\\nur\\%d\\pid\\%d",GP_NEWUSER_BAD_NICK,profileid);
		return;
	}
	if(!nameValid(nick,false)) {
		formatSend(sd,true,0,"\\nur\\%d",GP_NEWUSER_BAD_NICK);
		return;
	}
	if(uniquenick[0] != 0) {
		profileid = getProfileIDFromUniquenick(conn,uniquenick);
		if(profileid != 0) {
			formatSend(sd,true,0,"\\nur\\%d",GP_NEWUSER_UNIQUENICK_INUSE);
			return;
		}
		if(!nameValid(uniquenick,false)) {
			formatSend(sd,true,0,"\\nur\\%d",GP_NEWUSER_UNIQUENICK_INVALID);
			return;
		}
	}
	if(uniquenick[0] != 0) {
		profileid = makeNewProfileWithUniquenick(conn,nick,uniquenick,userid);
	} else {
		profileid = makeNewProfile(conn,nick,userid);
	}
	if(profileid != 0) {
		formatSend(sd,true,0,"\\nur\\0\\pid\\%d",profileid);
		return;
	}
		
}
void searchUsers(int sd, char *buff) {
	int searchnum = 0;
	int sesskey = 0;
	char nick[GP_NICK_LEN],email[GP_EMAIL_LEN],firstname[GP_FIRSTNAME_LEN],lastname[GP_LASTNAME_LEN],icquin[GP_AIMNAME_LEN],skip[GP_AIMNAME_LEN],uniquenick[GP_NICK_LEN];
	char *hideemail = "[hidden]";
	char *trueemail;
	int len = 1024;
	char tbuff[512];
	char *query;
	bool emailsearch = false;
	int namespaceid = find_paramint("namespaceid",buff);
	memset(&icquin,0,sizeof(icquin));
	if(find_param("email", buff, email, sizeof(email))) {
		mysql_real_escape_string(conn,email,email,strlen(email));
		len += strlen(email);
		searchnum++;
		emailsearch = true;
	}
	if(find_param("uniquenick", buff, uniquenick, sizeof(uniquenick))) {
		mysql_real_escape_string(conn,uniquenick,uniquenick,strlen(uniquenick));
		len += strlen(uniquenick);
		searchnum++;
	}
	if(find_param("nick", buff, nick, sizeof(nick))) {
		mysql_real_escape_string(conn,nick,nick,strlen(nick));
		len += strlen(nick);
		searchnum++;
	}
	if(find_param("firstname", buff, firstname, sizeof(firstname))) {
		mysql_real_escape_string(conn,firstname,firstname,strlen(firstname));
		len += strlen(firstname);
		searchnum++;
	}
	if(find_param("lastname", buff, lastname, sizeof(lastname))) {
		mysql_real_escape_string(conn,lastname,lastname,strlen(lastname));
		len += strlen(lastname);
		searchnum++;
	}
	if(find_param("icquin", buff, icquin, sizeof(icquin))) {
		mysql_real_escape_string(conn,icquin,icquin,strlen(icquin));
		len += strlen(icquin);
		searchnum++;
	}
	if(searchnum == 0) {
	       sendError(sd,"Error recieving request");
	       return;	
	}
	query = (char *)malloc(len);
	sprintf_s(query,len,"SELECT `profileid`,`nick`,`firstname`,`lastname`,`email`,`uniquenick`,`publicmask` FROM `GameTracker`.`profiles` INNER JOIN `GameTracker`.`users` ON `GameTracker`.`profiles`.`userid` = `GameTracker`.`users`.`userid` WHERE ");
	if(email[0] != 0) {
		sprintf_s(tbuff,sizeof(tbuff),"`email` = '%s'",email);
		if(--searchnum > 1) {
			strcat(tbuff, " AND ");
		}
		strcat(query,tbuff);
		if(strlen(query) > (len/2)) {
			len *= 2;
			query = (char *)realloc(query,len);
		}
	}
	if(nick[0] != 0) {
		sprintf_s(tbuff,sizeof(tbuff),"`nick` = '%s'",nick);
		if(--searchnum > 1) {
			strcat(tbuff, " AND ");
		}
		strcat(query,tbuff);
		if(strlen(query) > (len/2)) {
			len *= 2;
			query = (char *)realloc(query,len);
		}
	}
	if(firstname[0] != 0) {
		sprintf_s(tbuff,sizeof(tbuff),"`firstname` = '%s'",firstname);
		if(--searchnum > 1) {
			strcat(tbuff, " AND ");
		}
		strcat(query,tbuff);
		if(strlen(query) > (len/2)) {
			len *= 2;
			query = (char *)realloc(query,len);
		}
	}
	if(lastname[0] != 0) {
		sprintf_s(tbuff,sizeof(tbuff),"`lastname` = '%s'",lastname);
		if(--searchnum > 1) {
			strcat(tbuff, " AND ");
		}
		strcat(query,tbuff);
		if(strlen(query) > (len/2)) {
			len *= 2;
			query = (char *)realloc(query,len);
		}
	}
	if(uniquenick[0] != 0) {
		sprintf_s(tbuff,sizeof(tbuff),"`uniquenick` = '%s'",uniquenick);
		if(--searchnum > 1) {
			strcat(tbuff, " AND ");
		}
		strcat(query,tbuff);
		if(strlen(query) > (len/2)) {
			len *= 2;
			query = (char *)realloc(query,len);
		}
	}
	if(icquin[0] != 0) {
		sprintf_s(tbuff,sizeof(tbuff),"`icquin` = '%d'",atoi(icquin));
		strcat(query,tbuff);
	}
//	strcat(query," LIMIT 0,10");
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}
	res = mysql_store_result(conn);
	int u_pid,u_hideemail;
	memset(query,0,len);
	while ((row = mysql_fetch_row(res)) != NULL) {
		u_pid = atoi(row[0]);
		sprintf_s(nick,sizeof(nick),"%s",row[1]);
		sprintf_s(firstname,sizeof(firstname),"%s",row[2]);
		sprintf_s(lastname,sizeof(lastname),"%s",row[3]);
		sprintf_s(email,sizeof(email),"%s",row[4]);
		if(row[5] != NULL) {
			sprintf_s(uniquenick,sizeof(uniquenick),"%s",row[5]);
		} else uniquenick[0] = 0;
		u_hideemail = ~atoi(row[6])&GP_MASK_EMAIL;
		if(u_hideemail != 0 && !emailsearch) {
			sprintf_s(email,sizeof(email),"[hidden]");
		}
		sprintf_s(tbuff,sizeof(tbuff),"\\bsr\\%d\\nick\\%s\\firstname\\%s\\lastname\\%s\\email\\%s\\uniquenick\\%s\\namespaceid\\%d",u_pid,nick,firstname,lastname,email,uniquenick,namespaceid);
		if(strlen(tbuff) > (len/2)) {
			len *= 2;
			query = (char *)realloc(query,len);			
		}
		strcat(query,tbuff);
   	}
	strcat(query,"\\bsrdone\\");
	formatSend(sd,true,0,"%s",query);
	end:
	mysql_free_result(res);
	free((void *)query);
}
void sendReverseBuddies(int sd,char *msg) {
	int len = 1024;
	char tbuff[512];
	#define addVal(x,y,z) if( z == 1 ||strlen(x)) { \
						sprintf_s(tbuff,sizeof(tbuff),"\\%s\\%s",y,x); \
					} \
					if(strlen(tbuff) > (len/2)) { \
						len *= 2; \
						query = (char *)realloc(query,len); \
					} \
					strcat(query,tbuff);
//\others\\sesskey\0\profileid\121812147\namespaceid\1\partnerid\0\gamename\\final
	int profile = find_paramint("profileid",msg);
	char *query = (char *)malloc(len);
sprintf_s(query,len,"SELECT `GameTracker`.`profiles`.`profileid`,`uniquenick`,`email`,`firstname`,`lastname`,`nick`,`publicmask` FROM `Presence`.`buddies` INNER JOIN `GameTracker`.`profiles` ON `Presence`.`buddies`.`profileid` = `GameTracker`.`profiles`.`profileid` INNER JOIN `GameTracker`.`users` ON `GameTracker`.`users`.`userid` = `GameTracker`.`profiles`.`userid` WHERE `Presence`.`buddies`.`targetid` = %d AND `GameTracker`.`users`.`deleted` = 0 AND `GameTracker`.`profiles`.`deleted` = 0",profile);
	printf("query is: %s\n",query);
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		return;
	}
	memset(query,0,len);
	strcat(query,"\\otherslist\\");
	res = mysql_store_result(conn);
	while ((row = mysql_fetch_row(res)) != NULL) {
		addVal(row[0],"o",1)
		addVal(row[5],"nick",1)
		addVal(row[3],"first",1)
		addVal(row[4],"last",1)
		if(atoi(row[6])&GP_MASK_EMAIL) {
			addVal(row[2],"email",1)
		} else {
			addVal("[hidden]","email",1)
		}
		addVal(row[1],"uniquenick",0)
	}
	mysql_free_result(res);
	strcat(query,"\\odone\\");
	formatSend(sd,true,0,"%s",query);
	free((void *)query);
	#undef addVal
	
}
void sendError(int sd, char *msg) {
//\error\\errmsg\Error receiving request\final\ /
	formatSend(sd,true,0,"\\error\\\\errmsg\\%s",msg);
        close(sd);
}
