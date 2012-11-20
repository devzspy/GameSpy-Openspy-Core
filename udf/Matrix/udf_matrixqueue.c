#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>		/* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#define strmov(a,b) stpcpy(a,b)
#define bzero(a,b) memset(a,0,b)
#endif
#endif
#include <mysql.h>
#include <ctype.h>
#include <stdint.h>
#define _COMPILING_MATRIX_UDF
#define _JUST_STRUCTS
#define _NO_CPP
#include <peerchat/structs.h>
#ifdef _WIN32
/* inet_aton needs winsock library */
#pragma comment(lib, "ws2_32")
#endif

#ifdef HAVE_DLOPEN

#if !defined(HAVE_GETHOSTBYADDR_R) || !defined(HAVE_SOLARIS_STYLE_GETHOST)
static pthread_mutex_t LOCK_hostname;
#endif
#define getStringArg(i,dst) 	if(args->arg_type[i] != STRING_RESULT) { \
			\
} else { \
	if(args->args[i] != NULL) { \
		if(strlen((char *)args->args[i]) < sizeof(dst)) { \
			strncpy(dst,args->args[i],args->lengths[i]); \
		} \
	} \
} 
#define getIntArg(i,dst) if(args->arg_type[i] == INT_RESULT) { \
	dst = *(int *)args->args[i];\
}

char ip[24] = "127.0.0.1\0";
my_bool matrixqueue_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
longlong matrixqueue(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		    char *error);
int queueTypeArgs(char type);
void sendAddOperMsg(int profileid, int rightsmask) {
	
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		exit(-1);
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,&si_other.sin_addr)) {
		return;
	}
	msgAddOper msg;
	msg.profileid = profileid;
	msg.rightsmask = rightsmask;
	int len = sizeof(msgAddOper)+1;
	buf[0] = (char)EQueueID_AddOper;
	memcpy((void *)(buf+1),(char *)&msg,sizeof(msgAddOper));
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
void sendDelOperMsg(int profileid) {
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		return;
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,(const char *)&si_other.sin_addr)) {
		return;
	}
	msgAddOper msg;
	msg.profileid = profileid;
	int len = sizeof(msgDelOper)+1;
	buf[0] = (char)EQueueID_DelOper;
	memcpy((void *)(buf+1),(char *)&msg,sizeof(msgDelOper));
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
void sendDelUserMode(int profileid) {
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		return;
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,(const char *)&si_other.sin_addr)) {
		return;
	}
	int len = sizeof(int)+sizeof(char);
	buf[0] = (char)EQueueID_DelUserMode;
	int *pid = (int *)(buf+1);
	*pid = profileid;
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
void sendSetUserMode(UDF_ARGS *args) {
	//arg 0 = id, 1 = chanmask, comment,expires(int),hostmask,machineid,modeflags(int),profileid,setbyhost,setbynick,setbypid,setondate(int)
	userMode usermode;
	memset(&usermode,0,sizeof(userMode));
	getStringArg(1,usermode.chanmask)
	getStringArg(2,usermode.comment)
	getIntArg(3,usermode.expires)
	getStringArg(4,usermode.hostmask)
	getStringArg(5,usermode.machineid)
	getIntArg(6,usermode.modeflags)
	getIntArg(7,usermode.profileid)
	getStringArg(8,usermode.setbyhost)
	getStringArg(9,usermode.setbynick)
	getIntArg(10,usermode.setbypid)
	getIntArg(11,usermode.setondate)
	getIntArg(12,usermode.usermodeid)
	usermode.isGlobal = 1;
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		return;
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,(const char *)&si_other.sin_addr)) {
		return;
	}
	int len = sizeof(userMode)+1;
	buf[0] = (char)EQueueID_SetUserMode;
	memcpy((void *)(buf+1),(char *)&usermode,sizeof(userMode));
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
void sendSetChanProps(UDF_ARGS *args) {
	//arg 0 = id, 1 = chanmask, chankey, comment, entrymsg, expires(int), groupname, limit(int), modes, onlyowner(int), setbynick, setbyhost, setbypid(int), topic,setondate(int), kickexisting
	msgSetChanProps msg;
	memset(&msg,0,sizeof(msgSetChanProps));
	getStringArg(1,msg.props.chanmask)
	getStringArg(2,msg.props.chankey)
	getStringArg(3,msg.props.comment)
	getStringArg(4,msg.props.entrymsg)
	getIntArg(5,msg.props.expires)
	getStringArg(6,msg.props.groupname)
	getIntArg(7,msg.props.limit)
	getStringArg(8,msg.props.modes)
	getIntArg(9,msg.props.onlyowner)
	getStringArg(10,msg.props.setbynick)
	getStringArg(11,msg.props.setbyhost)
	getIntArg(12,msg.props.setbypid)
	getStringArg(13,msg.props.topic)
	getIntArg(14,msg.props.setondate)
	getIntArg(15,msg.kickexisting)
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		return;
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,(const char *)&si_other.sin_addr)) {
		return;
	}
	int len = sizeof(msgSetChanProps)+sizeof(char);
	buf[0] = (char)EQueueID_SetChanProps;
	memcpy((void *)(buf+1),(char *)&msg,sizeof(msgSetChanProps));
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
void sendDelChanProps(UDF_ARGS *args) {
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		return;
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,(const char *)&si_other.sin_addr)) {
		return;
	}
	msgDelChanProps msg;
	memset(&msg,0,sizeof(msgDelChanProps));
	strncpy(msg.chanmask,(char *)args->args[1],args->lengths[1]);
	msg.kickexisting = *(int *)args->args[2];
	int len = sizeof(msgDelChanProps)+sizeof(char);
	buf[0] = (char)EQueueID_DelChanProps;
	memcpy((void *)(buf+1),(char *)&msg,sizeof(msgDelChanProps));
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
void sendChanClientMsg(UDF_ARGS *args, char add) {
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		return;
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,(const char *)&si_other.sin_addr)) {
		return;
	}
	chanGameClient msg;
	memset(&msg,0,sizeof(msgDelChanProps));
	strncpy(msg.chanmask,(char *)args->args[1],args->lengths[1]);
	msg.gameid = *(int *)args->args[2];
	int len = sizeof(chanGameClient)+sizeof(char);
	buf[0] = (char)add?EQueueID_AddChanClient:EQueueID_DelChanClient;
	memcpy((void *)(buf+1),(char *)&msg,sizeof(chanGameClient));
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
void sendAuthClient(UDF_ARGS *args) {
	struct sockaddr_in si_other;
	int sd, i, slen = sizeof(si_other);
	char buf[4096];
	if((sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1) {
		return;
	}
	memset((char *)&si_other,0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(QUEUEPORT);
	if(!inet_pton(si_other.sin_family,(const char *)&ip,(const char *)&si_other.sin_addr)) {
		return;
	}
	msgAuthClient msg;
	memset(&msg,0,sizeof(msgAuthClient));
	getIntArg(1,msg.userid)
	getIntArg(2,msg.profileid)
	getStringArg(3,msg.uniquenick)
	getIntArg(4,msg.sd)
	getIntArg(5,msg.sendnotice)
	int len = sizeof(msgAuthClient)+sizeof(char);
	buf[0] = (char)EQueueID_AuthClient;
	memcpy((void *)(buf+1),(char *)&msg,sizeof(msgAuthClient));
	if(sendto(sd, buf,len,0, (struct sockaddr_in *)&si_other, slen) == -1) {
		return;
	}
	close(sd);
}
longlong matrixqueue(UDF_INIT *initid __attribute__((unused)), UDF_ARGS *args,
                    char *is_null __attribute__((unused)),
                    char *error __attribute__((unused)))
{
  char type = *(char *)args->args[0];
  switch(type) {
	case EQueueID_AddOper: {
		sendAddOperMsg(*(int *)args->args[1],*(int *)args->args[2]);
		break;
	}
	case EQueueID_DelOper: {
		sendDelOperMsg(*(int *)args->args[1]);
		break;
	}
	case EQueueID_SetUserMode: {
		sendSetUserMode(args);
		break;
	}
	case EQueueID_DelUserMode: {
		sendDelUserMode(*(int *)args->args[1]);
		break;
	}
	case EQueueID_SetChanProps: {
		sendSetChanProps(args);
		break;
	}
	case EQueueID_DelChanProps: {
		sendDelChanProps(args);
		break;
	}
	case EQueueID_AddChanClient: {
		sendChanClientMsg(args,1);
		break;
	}
	case EQueueID_DelChanClient: {
		sendChanClientMsg(args,0);
		break;
	}
	case EQueueID_AuthClient:
		sendAuthClient(args);
		break;
  }
  return 0;
}

/*
  At least one of _init/_deinit is needed unless the server is started
  with --allow_suspicious_udfs.
*/
my_bool matrixqueue_init(UDF_INIT *initid __attribute__((unused)),
                        UDF_ARGS *args __attribute__((unused)),
                        char *message __attribute__((unused)))
{
  if (args->arg_count < 1)
  {
    strcpy(message,"matrixqueue needs atleast 1 argument");
    return 1;
  }
   if (args->args[0] == NULL) {
    strcpy(message,"matrixqueue needs atleast 1 argument");
    return 1;
   }
   if(args->arg_type[0] != INT_RESULT) {
    strcpy(message,"matrixqueue requires that argument 1 be an integer");
    return 1;
  }
  char type = *(char *)args->args[0];
  int argnum = queueTypeArgs(type); 
  if(args->arg_count < argnum +1) {
    char error[80];
    snprintf((char *)&error,sizeof(error),"matrixqueue id %d requires %d arguments",type,argnum+1); 
    strcpy(message,(const char *)&error);
    return 1;
  }
  return 0;
}

int queueTypeArgs(char type) {
	switch(type) {
		case EQueueID_AddOper:
			return 2; //profileid, rightsmask
		case EQueueID_DelOper:
			return 1; //profileid
		case EQueueID_SetUserMode:
			return 11; //chanmask, comment,expires(int),hostmask,machineid,modeflags(int),profileid,setbyhost,setbynick,setbypid,setondate(int),usermodeid
		case EQueueID_DelUserMode:
			return 1; //usermodeid
		case EQueueID_SetChanProps:
			return 15; //chanmask, chankey, comment, entrymsg, expires(int), groupname, limit(int), modes, onlyowner(int), setbynick, setbyhost, setbypid(int), topic,setondate(int), kickexisting
		case EQueueID_DelChanProps:
			return 2; //chanmask, kickexisting
		case EQueueID_AddChanClient:
			return 2;
		case EQueueID_DelChanClient:
			return 2;
		case EQueueID_AuthClient:
			return 5; //userid, profileid, uniquenick, sd, sendnotice
	}
	return 0;
}

#endif /* HAVE_DLOPEN */

