#include "mysql_helpers.h"
void clearResults(MYSQL *sql) {
	MYSQL_RES *res;
	while(mysql_more_results(sql)) {
		if(mysql_next_result(sql)) {
			res = mysql_use_result(sql);
			mysql_free_result(res);
		}
	}
}
int getUserIDFromEmail(MYSQL *sql, char *email) {
   MYSQL_RES *res;
   MYSQL_ROW row;
   mysql_ping(sql);
   int userid = 0;
   int len = 1024 + strlen(email)+2;
   char *query = (char *)malloc(len);
   if(query == NULL) return 0;
   mysql_real_escape_string(sql,email,email,strlen(email));
   sprintf_s(query,len,"SELECT  `userid` FROM `GameTracker`.`users` WHERE `email` = '%s' AND `deleted` = 0",email);
   if (mysql_query(sql, query)) {
      fprintf(stderr, "%s\n", mysql_error(sql));
	return 0;
   }
   res = mysql_store_result(sql);
   while ((row = mysql_fetch_row(res)) != NULL) {
	userid = atoi(row[0]);
   }
   mysql_free_result(res);
   end:
   free((void *)query);
   return userid;
}
int getProfileUserID(MYSQL *sql, int profileid) {
   MYSQL_RES *res;
   MYSQL_ROW row;
   char query[256];
   mysql_ping(sql);
   int userid = 0;
   sprintf_s(query,sizeof(query),"SELECT  `userid` FROM `GameTracker`.`profiles` WHERE `profileid` = '%d' AND `deleted` = 0",profileid);
   if (mysql_query(sql, query)) {
      fprintf(stderr, "%s\n", mysql_error(sql));
	return 0;
   }
   res = mysql_store_result(sql);
   int num_rows = mysql_num_rows(res);
   if(num_rows == 0) goto end;
   while ((row = mysql_fetch_row(res)) != NULL) {
	userid = atoi(row[0]);
   }
   end:
   mysql_free_result(res);
   return userid;
}
bool tryPassword(MYSQL *sql, int userid, char *pass) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	bool found = false;
	int len = 1024 + strlen(pass);
	char *query = (char *)malloc(len);
	if(query == NULL) return false;
	mysql_real_escape_string(sql,pass,pass,strlen(pass));
	sprintf_s(query,len,"SELECT 1 FROM `GameTracker`.`users` WHERE `userid` = %d AND `password` = '%s' AND `deleted` = 0",userid,pass);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return false;
	}
	res = mysql_store_result(sql);
	found = mysql_num_rows(res) != 0;
	mysql_free_result(res);
	free((void *)query);
	return found;
}
bool tryPasswordMD5(MYSQL *sql, int userid, char *pass) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	bool found = false;
	int len = 1024 + strlen(pass);
	char *query = (char *)malloc(len);
	if(query == NULL) return false;
	mysql_real_escape_string(sql,pass,pass,strlen(pass));
	sprintf_s(query,len,"SELECT 1 FROM `GameTracker`.`users` WHERE `userid` = %d AND md5(`password`) = '%s' AND `deleted` = 0",userid,pass);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return false;
	}
	res = mysql_store_result(sql);
	found = mysql_num_rows(res) != 0;
	mysql_free_result(res);
	free((void *)query);
	return found;
}
int getProfileIDFromNickEmail(MYSQL *sql, char *nick, char *email) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	int profileid = 0;
	int len = 1024 + strlen(nick) + strlen(email);
	char *query = (char *)malloc(len);
	if(query == NULL) return 0;
	mysql_real_escape_string(sql,nick,nick,strlen(nick));
	mysql_real_escape_string(sql,email,email,strlen(email));
	sprintf_s(query,len,"SELECT `profileid` FROM `GameTracker`.`profiles` INNER JOIN `GameTracker`.`users` ON `GameTracker`.`profiles`.`userid` = `GameTracker`.`users`.`userid` WHERE `email` = '%s' AND `nick` = '%s' AND `GameTracker`.`profiles`.`deleted` = 0",email,nick);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return 0;
	}
	res = mysql_store_result(sql);
	int num_rows = mysql_num_rows(res);
	if(num_rows == 0) goto end;
	while ((row = mysql_fetch_row(res)) != NULL) {
		profileid = atoi(row[0]);
	}
  	mysql_free_result(res);
	end:
	free((void *)query);	
	return profileid;
}
int makeNewProfile(MYSQL *sql, char *nick, int userid) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	int len = 1024 + strlen(nick);
	char *query = (char *)malloc(len);
	int profileid = 0;
	mysql_real_escape_string(sql,nick,nick,strlen(nick));
	sprintf_s(query,len,"INSERT INTO `GameTracker`.`profiles` (`userid`,`nick`) VALUES (%d,\"%s\");",userid,nick);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return 0;
	}
	mysql_query(sql,"SELECT LAST_INSERT_ID()");
	res = mysql_store_result(sql);
	while ((row = mysql_fetch_row(res)) != NULL) {
		profileid = atoi(row[0]);
	}
  	mysql_free_result(res);
	free((void *)query);	
	return profileid;
}
bool isUserIDEmailHidden(MYSQL *sql, int userid) {
    return ~getPublicMask(sql,userid)&GP_MASK_EMAIL;
}
uint32_t getPublicMask(MYSQL *sql, int userid) {
   int publicmask = 0;
   MYSQL_RES *res;
   MYSQL_ROW row;
   char query[256];
   mysql_ping(sql);
   sprintf_s(query,sizeof(query),"SELECT  `publicmask` FROM `GameTracker`.`users` WHERE `userid` = '%d' AND `deleted` = 0",userid);
   if (mysql_query(sql, query)) {
      fprintf(stderr, "%s\n", mysql_error(sql));
	return 0;
   }
   res = mysql_store_result(sql);
   int num_rows = mysql_num_rows(res);
   if(num_rows == 0) goto end;
   while ((row = mysql_fetch_row(res)) != NULL) {
	publicmask = atoi(row[0]);
   }
   end:
   mysql_free_result(res);
   return publicmask;	
}
void getUserIDPass(MYSQL *sql, int userid, char *dst, int dstlen) {
   bool hidden = false;
   MYSQL_RES *res;
   MYSQL_ROW row;
   char query[256];
   memset(dst,0,dstlen);
   mysql_ping(sql);
   sprintf_s(query,sizeof(query),"SELECT `password` FROM `GameTracker`.`users` WHERE `userid` = '%d' AND `deleted` = 0",userid);
   if (mysql_query(sql, query)) {
      fprintf(stderr, "%s\n", mysql_error(sql));
	return;
   }
   res = mysql_store_result(sql);
   int num_rows = mysql_num_rows(res);
   if(num_rows == 0) goto end;
   while ((row = mysql_fetch_row(res)) != NULL) {
	strncpy(dst,row[0],strlen(row[0])%dstlen);
   }
   end:
   mysql_free_result(res);
   return;
}
void getProfileIDPass(MYSQL *sql, int profileid, char *dst, int dstlen) {
	int userid = getProfileUserID(sql,profileid);
	getUserIDPass(sql,userid,dst,dstlen);
}
int getProfileIDFromUniquenick(MYSQL *sql, char *uniquenick) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	int profileid = 0;
	int len = 1024 + strlen(uniquenick);
	char *query = (char *)malloc(len);
	if(query == NULL) return 0;
	mysql_real_escape_string(sql,uniquenick,uniquenick,strlen(uniquenick));
	sprintf_s(query,len,"SELECT `profileid` FROM `GameTracker`.`profiles` WHERE `uniquenick` = '%s' AND `deleted` = 0",uniquenick);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return 0;
	}
	res = mysql_store_result(sql);
	int num_rows = mysql_num_rows(res);
	if(num_rows == 0) goto end;
	while ((row = mysql_fetch_row(res)) != NULL) {
		profileid = atoi(row[0]);
	}
  	mysql_free_result(res);
	end:
	free((void *)query);	
	return profileid;
}
int makeNewProfileWithUniquenick(MYSQL *sql, char *nick, char *uniquenick, int userid) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	int len = 1024 + strlen(nick);
	char *query = (char *)malloc(len);
	if(!query) return 0;
	int profileid = 0;
	mysql_real_escape_string(sql,nick,nick,strlen(nick));
	mysql_real_escape_string(sql,uniquenick,uniquenick,strlen(uniquenick));
	sprintf_s(query,len,"INSERT INTO `GameTracker`.`profiles` (`userid`,`nick`,`uniquenick`) VALUES (%d,\"%s\",\"%s\")",userid,nick,uniquenick);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		free((void *)query);
		return 0;
	}
	mysql_query(sql,"SELECT LAST_INSERT_ID()");
	res = mysql_store_result(sql);
	while ((row = mysql_fetch_row(res)) != NULL) {
		profileid = atoi(row[0]);
	}
  	mysql_free_result(res);
	free((void *)query);	
	return profileid;
}
bool validProfileID(MYSQL *sql, int profileid) {
	int userid = getProfileUserID(sql,profileid);
	return userid!=0;
}
int registerUser(MYSQL *sql,char *email,char *pass) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	int len = 1024 + strlen(email) + strlen(pass);
	char *query = (char *)malloc(len);
	if(!query) return 0;
	int userid = 0;
	mysql_real_escape_string(sql,email,email,strlen(email));
	mysql_real_escape_string(sql,pass,pass,strlen(pass));
	sprintf_s(query,len,"INSERT INTO `GameTracker`.`users` (`email`,`password`) VALUES (\"%s\",\"%s\")",email,pass);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		free((void *)query);
		return 0;
	}
	mysql_query(sql,"SELECT LAST_INSERT_ID()");
	res = mysql_store_result(sql);
	while ((row = mysql_fetch_row(res)) != NULL) {
		userid = atoi(row[0]);
	}
  	mysql_free_result(res);
	free((void *)query);	
	return userid;
}
bool getProfileInfo(MYSQL *sql,int profileid,GPIInfoCache *info) {
	char query[1024];
	MYSQL_RES *res;
	MYSQL_ROW row;
	int num_rows = 0;
	memset(info,0,sizeof(GPIInfoCache));
	info->profileid = profileid;
	#define setString(a,b) if(b != NULL && strlen(b) > 1) {\
				 a = (char *)malloc(strlen(b) + 1); \
				 strcpy(a,b);\
				} 
	sprintf_s(query,sizeof(query),"SELECT `nick`,`uniquenick`,`email`,`firstname`,`lastname`,`homepage`,`icquin`,`zipcode`,`countrycode`,`lon`,`lat`,`birthday`,`sex`,`publicmask`,`aimname`,`pic`,`ooc`,`ind`,`inc`,`mar`,`chc`,`i1`,`o1`,`conn`,`GameTracker`.`profiles`.`userid`,`place`,DAY(birthday), YEAR(birthday), MONTH(birthday), `aimname` FROM `GameTracker`.`profiles` INNER JOIN `GameTracker`.`users` ON `GameTracker`.`profiles`.`userid` = `GameTracker`.`users`.`userid` WHERE `profileid` = %d AND `GameTracker`.`profiles`.`deleted` = 0 AND `GameTracker`.`users`.`deleted` = 0",profileid);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return false;
	}
	res = mysql_store_result(sql);
	num_rows = mysql_num_rows(res);
	if ((row = mysql_fetch_row(res)) != NULL) {
		setString(info->nick,row[0])
		setString(info->uniquenick,row[1])
		setString(info->email,row[2])
		setString(info->firstname,row[3])
		setString(info->lastname,row[4])
		setString(info->homepage,row[5])
		info->icquin = atoi(row[6]);
		sprintf_s(info->zipcode,sizeof(info->zipcode),"%s",row[7]);
		sprintf_s(info->countrycode,sizeof(info->countrycode),"%s",row[8]);
		info->longitude = atof(row[9]);
		info->latitude = atof(row[10]);
		info->sex = (GPEnum)atoi(row[12]);
		info->publicmask = atoi(row[13]);
		setString(info->aimname,row[14])
		info->pic = atoi(row[15]);
		info->occupationid = atoi(row[16]);
		info->industryid = atoi(row[17]);
		info->incomeid = atoi(row[18]);
		info->marriedid = atoi(row[19]);
		info->childcount = atoi(row[20]);
		info->interests1 = atoi(row[21]);
		info->ownership1 = atoi(row[22]);
		info->conntypeid = atoi(row[23]);
		info->userid = atoi(row[24]);
		sprintf_s(info->place,sizeof(info->place),"%s",row[25]);
		info->birthday = atoi(row[26]);
		info->birthyear = atoi(row[27]);
		info->birthmonth = atoi(row[28]);
		setString(info->aimname,row[29])
	}
	mysql_free_result(res);	
	#undef setString
	return num_rows>0;
}
void updateUserProfile(MYSQL *sql, GPIInfoCache *info) {
	char *query;
	int len = 2048;
	#define escapeString(x) if(x != NULL) { \
					int elen = (strlen(x)*2)+1; \
					char *escapenick = (char *)malloc(elen); \
					len += elen + 1; \
					mysql_real_escape_string(sql,escapenick,x,strlen(x));\
					free((void *)x); \
					x = (char *)escapenick; \
				} 
	#define escapeStringStack(x,v) 	char * v = NULL; \
					if(x[0] != 0) { \
						v = (char *)malloc((strlen(x)*2)+1); \
						if(v != NULL) { \
							mysql_real_escape_string(sql,v,x,strlen(x));\
 						} \
					}
	#define setString(x,y,z) 	if(y != NULL) { \
					sprintf_s(tmp,sizeof(tmp),"`%s` = '%s'",x,y); \
				} else {  \
					sprintf_s(tmp,sizeof(tmp),"`%s` = ''",x); \
				} \
				strcat(query,tmp); \
				if(z == 1) strcat(query, ",");
	#define setInt(x,y,z) sprintf_s(tmp,sizeof(tmp),"`%s` = '%d'",x,y); \
				strcat(query,tmp); \
				if(z == 1) strcat(query, ",");
	#define setFloat(x,y,z) sprintf_s(tmp,sizeof(tmp),"`%s` = '%f'",x,y); \
				strcat(query,tmp); \
				if(z == 1) strcat(query, ",");
	escapeString(info->firstname)
	escapeString(info->lastname)
	escapeString(info->nick)
	escapeString(info->uniquenick)
	escapeString(info->homepage)
	escapeString(info->aimname)
	escapeStringStack(info->zipcode,zipcode);
	escapeStringStack(info->countrycode,countrycode);
	escapeStringStack(info->place,place);
	query = (char *)malloc(len);
	memset(query,0,len);
	char tmp[128];
	sprintf_s(query,len,"UPDATE `GameTracker`.`profiles`,`GameTracker`.`users` SET ");
	setString("firstname",info->firstname,1)
	setString("lastname",info->lastname,1)
	setString("nick",info->nick,1)
	setString("homepage",info->homepage,1)
	setString("aimname",info->aimname,1)
	setString("zipcode",zipcode,1)
	setInt("publicmask",info->publicmask,1)
	setInt("pic",info->pic,1)
	setInt("sex",info->sex,1)
	setInt("ind",info->industryid,1)
	setInt("inc",info->incomeid,1)
	setInt("mar",info->marriedid,1)
	setInt("chc",info->childcount,1)
	setInt("i1",info->interests1,1)
	setInt("o1",info->ownership1,1)
	setInt("conn",info->conntypeid,1)
	sprintf_s(tmp,sizeof(tmp),"`birthday` = '%04d-%02d-%02d'",info->birthyear,info->birthmonth,info->birthday); 
	strcat(query,tmp); 

	sprintf_s(tmp,sizeof(tmp)," WHERE `profileid` = %d AND `GameTracker`.`users`.`userid` = %d",info->profileid,info->userid);
	strcat(query,tmp);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
	}
	free((void *)query);
	if(zipcode != NULL)
	free((void *)zipcode);
	if(countrycode != NULL)
	free((void *)countrycode);
	if(place != NULL)
	free((void *)place);
	#undef escapeString
	#undef escapeStringStack
	#undef setString
	return;
}
void addAuthCdKey(MYSQL *sql, int profileid, char *cdkey, gameInfo *game) {
	char cdkeyesc[(GP_CDKEY_LEN*2)+1];
	if(game == NULL) return;
	mysql_real_escape_string(sql,cdkeyesc,cdkey,strlen(cdkey));
	char query[256 + (GP_CDKEY_LEN*2)+1];
	sprintf_s(query,sizeof(query),"INSERT INTO `GameTracker`.`authedcdkeys` (`gameid`,`profileid`,`cdkey`) VALUES (%d,%d,\"%s\")",game->id,profileid,cdkeyesc);
	if (mysql_query(sql, query)) {
		fprintf(stderr, "%s\n", mysql_error(sql));
	}
}
