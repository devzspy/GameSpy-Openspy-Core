#ifndef _HANDLER_SERVERBROWSING_INC
#define _HANDLER_SERVERBROWSING_INC
#include "main.h"
#define _NO_CPP
#define _JUST_STRUCTS
#include <peerchat/structs.h>
#include <qr/structs.h>
#undef _NO_CPP
#undef _JUST_STRUCTS
typedef struct {
	uint8_t *querygame;
	uint8_t *gamename;
	uint8_t *filter;
	uint8_t *fieldlist;
	uint8_t challenge[9];
	uint32_t srcip;
	uint32_t maxservers;
	uint32_t fromgamever;
	uint8_t listversion;
	uint8_t encodingversion;
	uint32_t options;
	uint32_t fromip;
	int sd;
	struct sockaddr_in peer;
	unsigned char   encxkeyb[261];
	time_t lastKeepAlive;
	gameInfo *game;
	gameInfo *queryGame;
	bool	cryptHeaderSent;
	uint32_t headerLen;
	bool terminated;
} handlerOptions;
#define CRYPTCHAL_LEN 10
#define SERVCHAL_LEN 25
void *processConnection(threadOptions *options);
void handleListRequest(uint8_t *buff, uint32_t len, handlerOptions *handleroptions);
void handleInfoRequest(uint8_t *buff, uint32_t len, handlerOptions *handleroptions);
void sendGroups(handlerOptions *options);
void addGroupBuff(char **buff,int *len, char *fieldList, MYSQL_ROW row);
void sendServers(handlerOptions *options);
char *findServerValue(char *name,serverList list);
void addServerBuff(char **buff,int *len,handlerOptions *options, serverList slist);
void setupCryptHeader(handlerOptions *options, uint8_t **dst, uint32_t *len);
void InitCryptKey(char *challenge, char *queryfromkey,char *key, int keylen);
#endif
