#include "statsmanage.h"
#include "structs.h"
extern legacyStatsProcessor server;
char *mergeKeys(char *keys,int localid,persisttype_t type,int index,int pid) {
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
	mlen *= 2; //for mysql escaping
	mlen++;
	rkeys = (char *)malloc(mlen);
	if(rkeys == NULL) {
		mysql_free_result(res);
		return keys;
	}
	memset(rkeys,0,mlen);
	int i = 0;
	char sbuff[1024],kbuff[1024];
	while(find_param(i,curkeys,sbuff,sizeof(sbuff))) {
		if(!find_param(sbuff,keys,kbuff,sizeof(kbuff))) { //not found in key buff
			find_param(sbuff,curkeys,kbuff,sizeof(kbuff));
		}
		if(strlen(kbuff) > 0) {
			strcat(rkeys,"\\\\");
			strcat(rkeys,sbuff);
			strcat(rkeys,"\\\\");
			strcat(rkeys,kbuff);
		}
		i+=2;
	}
	i = 0;
	while(find_param(i,keys,sbuff,sizeof(sbuff))) {
		if(!find_param(sbuff,curkeys,kbuff,sizeof(kbuff))) {
			find_param(sbuff,keys,kbuff,sizeof(kbuff));
			if(strlen(kbuff) > 0) {
				strcat(rkeys,"\\\\");
				strcat(rkeys,sbuff);
				strcat(rkeys,"\\\\");
				strcat(rkeys,kbuff);
			}
		}
		i+=2;
	}
	mysql_free_result(res);
	return rkeys;
}
bool shouldUpdate(int gameid, persisttype_t type, int index, int profileid) {
	MYSQL_RES *res;
	char query[256];
	bool retval = false;
	sprintf_s(query,sizeof(query),"SELECT 1 FROM `Persist`.`data` WHERE `type` = %d AND `profileid` = %d AND `index` = %d AND `gameid` = %d",type,profileid,index,gameid);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return false;
	}
	res = mysql_store_result(server.conn);
	retval = mysql_num_rows(res) > 0;
	mysql_free_result(res);
	return retval;
}
void getKeyString(int gameid, persisttype_t type, int index, int profileid, char *name, char *dst, int dstlen) {

	char query[256];
	MYSQL_RES *res;
	MYSQL_ROW row;
	sprintf_s(query,sizeof(query),"SELECT `data` FROM `Persist`.`data` WHERE `type` = %d AND `profileid` = %d AND `index` = %d",type,profileid,index);
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
		return;
	}
	char *tdata = NULL;
	res = mysql_store_result(server.conn);
	while(row = mysql_fetch_row(res)) {
		tdata = (char *)malloc(strlen(row[0]) + 1);
		strcpy(tdata,row[0]);
		break; 
	}
	find_param(name,tdata,dst,dstlen);
	mysql_free_result(res);
	free((void *)tdata);
}
int getKeyInt(int gameid, persisttype_t type, int index, int profileid, char *name) {
	char dst[256];
	getKeyString(gameid,type,index,profileid,name,dst,sizeof(dst));
	if(strlen(dst) > 0) {
		return atoi(dst);
	}
	return 0;
}
void setKeyInt(int gameid, persisttype_t type, int index, int profileid, char *name, int value) {
	char keyname[256];
	sprintf_s(keyname,sizeof(keyname),"%d",value);
	setKeyString(gameid,type,index,profileid,name,keyname);
}
void setKeyString(int gameid, persisttype_t type, int index, int profileid, char *name, char *value) {
	char *query;
	int klen = strlen(name) + strlen(value) + 4;
	char *keyname = (char *)malloc(klen);
	sprintf_s(keyname,klen,"\\%s\\%s",name,value);
	char *newkeys = mergeKeys(keyname,0,type,index,profileid);
	int mlen = strlen(newkeys) + 3 + strlen(keyname) + 256;
	query = (char *)malloc(mlen);
	int xlen = strlen(newkeys);
	for(int i=0;i<xlen;i++) {
		if(newkeys[i] == '\\') {
			newkeys[i] = '\x01';
		}
	}
	mysql_real_escape_string(server.conn,newkeys,newkeys,strlen(newkeys));
	for(int i=0;i<xlen;i++) {
		if(newkeys[i] == '\x01') {
			newkeys[i] = '\\';
		}
	}
	if(shouldUpdate(gameid, type, index, profileid)) {
		sprintf_s(query,mlen,"UPDATE `Persist`.`data` SET `data` = \"%s\" WHERE `profileid` = %d AND `index` = %d AND `gameid` = %d AND `type` = %d",newkeys,profileid,index,gameid,type);
	} else {
		sprintf_s(query,mlen,"INSERT INTO `Persist`.`data` (`data`,`profileid`,`index`,`gameid`,`type`) VALUES (\"%s\",%d,%d,%d,%d)",newkeys,profileid,index,gameid,type);
	}
	if(mysql_query(server.conn,query)) {
		fprintf(stderr,"%s\n",mysql_error(server.conn));
	}
	free((void *)newkeys);
	free((void *)keyname);
	free((void *)query);
}
