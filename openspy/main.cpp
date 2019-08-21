#include "main.h"
#include "config.h"
char *mysql_server = NULL;
char *mysql_user = NULL;
char *mysql_password = NULL;
char *mysql_database = NULL;
gameInfo *games;
int g_numGames;
Config *config;
std::list <CModule *> moduleList;

void startModule(configVar *info);

bool sendModuleMessage(char *sendername, char *name, void *data, int len) {
	CModule *mod;
	std::list<CModule *>::iterator iterator = moduleList.begin();
	while(iterator != moduleList.end()) {
		mod = *iterator;
		if(strcasecmp(mod->getModInfo()->name,name) == 0) 
			return mod->sendModuleMessage(sendername,data,len);
		iterator++;
	}	
	return false;
}
void logMessage(char *sendername, int level, char *msg, ...) {
	char fmsg[256];
	va_list args;
	va_start(args,msg);
	vsprintf(fmsg,msg,args);
	fprintf(stderr,"%s(%d): %s",sendername,level,fmsg);
	va_end(args);
}
void startModules() {
	CModule *mod;
	std::list<CModule *>::iterator iterator = moduleList.begin();
	while(iterator != moduleList.end()) {
		mod = *iterator;
		printf("Starting module: %s\n",mod->getModInfo()->name);
		mod->start();
		iterator++;
	}
}
gameInfo *findGameInfoByName(char *name) {
	int i = 0;
	while(games[i].name != NULL) {
		if(strcasecmp(games[i].name,name) == 0) {
			return &games[i];
		}
	i++;
	}
	return NULL;
}
gameInfo *findGameInfoByID(int gameid) {
	int i = 0;
	while(games[i].name != NULL) {
		if(games[i].id == gameid) {
			return &games[i];
		}
	i++;
	}
	return NULL;
}
void addPushKeys(gameInfo *game,char *keylist, char *keylisttype) {
	char tempkey[256];
	int keytype = 0;
	game->numPushKeys = countchar(keylist, '\\');
	game->pushKeys = (keyData *)malloc(sizeof(keyData) * (game->numPushKeys));
	memset((keyData *)game->pushKeys,0,sizeof(keyData) * (game->numPushKeys));
	int i=0;
	while(find_param(i,keylist,(char *)&tempkey,sizeof(tempkey))) {
		game->pushKeys[i].name = (char *)calloc(1,strlen(tempkey)+1);
		strcpy(game->pushKeys[i++].name,tempkey);
	}
	for(i = 0;i<game->numPushKeys;i++) {
		game->pushKeys[i].type = find_paramint(i,keylisttype);
	}
}
int main() {
#ifdef _WIN32
	WSADATA wsdata;
	WSAStartup(MAKEWORD(2,2),&wsdata);
#endif
	MYSQL_RES *res;
	MYSQL_ROW row;
	MYSQL *conn;
	modLoadOptions options;
	conn = mysql_init(NULL);
	config = new Config("openspy.cfg");
	mysql_server = config->getRootString("mysql_server");
	mysql_user = config->getRootString("mysql_user");
	mysql_password = config->getRootString("mysql_password");
	mysql_database = config->getRootString("mysql_database");
	if(mysql_server == NULL || mysql_user == NULL || mysql_password == NULL || mysql_database == NULL) {
		printf("Missing mysql_server, mysql_user, mysql_database, or mysql_password variable from config file\n");
		return -1;
	}
   	if (!mysql_real_connect(conn, mysql_server,
	    mysql_user, mysql_password, mysql_database, 0, NULL, CLIENT_MULTI_RESULTS)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	if (mysql_query(conn, "SELECT `id`,`gamename`,`secretkey`,`queryport`,`disabledservices`,`backendflags`,`keylist`,`keytypelist` FROM `GameTracker`.`games`")) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	res = mysql_store_result(conn);
	int num_rows = mysql_num_rows(res);
	games = (gameInfo *)malloc(sizeof(gameInfo)*(num_rows+1));
	memset(games,0,sizeof(gameInfo)*(num_rows+1));
	g_numGames = num_rows;
	int i=0;
	int len = 0;
	while ((row = mysql_fetch_row(res)) != NULL) {
		games[i].id = atoi(row[0]);
		len = strlen(row[1])+1;
		games[i].name = (char *)malloc(len);
		memset(games[i].name,0,len);
		strcpy(games[i].name,row[1]);
		len = strlen(row[2])+1;
		games[i].secretkey = (char *)malloc(len);
		memset(games[i].secretkey,0,len);
   		strcpy(games[i].secretkey,row[2]);
		games[i].queryport = atoi(row[3]);
		games[i].servicesdisabled = atoi(row[4]);
		games[i].backendflags = atoi(row[5]);
		addPushKeys(&games[i++],row[6],row[7]);
        }
        mysql_free_result(res);
	mysql_close(conn);
	configVar *moduleArray = config->getRootArray("modules");
	if(moduleArray != NULL) {
		std::list<configVar *> varlist = config->getArrayVariables(moduleArray);
		std::list<configVar *>::iterator it = varlist.begin();
		configVar *v;
		while(it != varlist.end()) {
			v = *it;
			startModule(v);
			it++;
		}
	}
	startModules();
	for(;;) {
		sleep(1000);
	}
}
int getModuleConfInt(configVar *info, char *name) {
	return config->getArrayInt(info,name);
}
char *getModuleConfString(configVar *info, char *name) {
	return config->getArrayString(info,name);
}
void startModule(configVar *info) {
	char path[256];
	uint32_t g_bindip = config->getRootInteger("bindip");
	sprintf_s(path,sizeof(path),"modules/%s%s",info->name,MODULE_EXTENSION);
	modLoadOptions options;
	memset(&options,0,sizeof(modLoadOptions));
	options.mysql_server = mysql_server;
	options.mysql_user = mysql_user;
	options.mysql_password = mysql_password;
	char *db;
	if((db = config->getArrayString(info, "database")) == NULL) {
		db = mysql_database;
	}
	options.bindIP = config->getArrayInt(info,"bindip");
	if(options.bindIP == 0) options.bindIP = g_bindip;
	options.mysql_database = db;
	options.moduleArray = info;
	options.getConfInt = getModuleConfInt;
	options.getConfStr = getModuleConfString;
	options.games = games;
	options.totalgames = g_numGames;
	options.sendMsgProc = sendModuleMessage;
	options.logMessageProc = logMessage;
	options.gameInfoNameProc = findGameInfoByName;
	options.gameInfoIDProc = findGameInfoByID;
	moduleList.push_back(new CModule(path,&options));

}
