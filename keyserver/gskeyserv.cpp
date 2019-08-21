#include "gskeyserv.h"
modInfo moduleInfo = {"keyserver","Gamespy Key Server Clone"};

void *openspy_mod_run(modLoadOptions *options)
{
#ifdef _WIN32
  WSADATA wsdata;
  WSAStartup(MAKEWORD(2,2),&wsdata);
#endif
  int sd;
  int len;
  char buf[1024];
  char skey[16];
  char key[512];
  struct sockaddr si_other;
  socklen_t slen;
  struct sockaddr_in si_me;
  if((sd=socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP)) == -1)
   return NULL;
  memset((char *)&si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(KEYPORT);
  //int ip;
//  inet_pton(AF_INET, SERVIP,&ip);
  si_me.sin_addr.s_addr = options->bindIP;
  if(bind(sd,(struct sockaddr *)&si_me,sizeof(si_me)) == -1)
   return NULL;
  for(;;) {
    memset((char *)&buf,0, sizeof(buf));
    len = recvfrom(sd,(char *)&buf,sizeof(buf),0, (struct sockaddr *)&si_other, &slen);
    makeStringSafe((char *)&buf, sizeof(buf));
    if(len < 1) break;	
    gamespyxor(buf,len);
    memset(&key,0,sizeof(key));
    memset(&skey,0,sizeof(skey));
    if(!find_param("skey", buf, skey, sizeof(skey))) {
		continue;
    }
    if(find_param("resp", buf, key, sizeof(key))) {
		key[32] = 0;
    } else continue;
    sprintf_s(buf,sizeof(buf),"\\uok\\\\cd\\%s\\skey\\%s",key,skey);
    len = strlen(buf);
    gamespyxor(buf,len);
    sendto(sd,(char *)&buf,len,0,(struct sockaddr *)&si_other, slen);
  }
  return NULL;
}
modInfo *openspy_modInfo() {
	return &moduleInfo;
}
bool openspy_mod_query(char *sendmodule, void *data,int len) {
	return false;
}
