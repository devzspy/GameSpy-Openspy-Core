#include <openspy/config.h>
#include "handler.h"
#include "cfuncs.h"
extern legacyStatsProcessor server;

static void openlualibs(lua_State *l)
{
static const luaL_reg lualibs[] =
{
       	{ "base",       luaopen_base },
       	{ NULL,         NULL }
};
       	const luaL_reg *lib;
     		for (lib = lualibs; lib->func != NULL; lib++)
	{
               	lib->func(l);
               	lua_settop(l, 0);
       	}
}
bool file_exists(char *path) {
	if(FILE *fd = fopen(path,"r")) {
		fclose(fd);
		return true;
	}
	return false;
}
void registerCFuncs(lua_State *l) {

	struct CFuncTable {
		char *name;
		int (*mpFunc)(lua_State *);
	};
	struct CFuncTable ctable[] =
	{
	   {(char*)"chctest",chctest},
	   {(char*)"find_paramint", GetSSInt},
	   {(char*)"find_param", GetSSString},
	   {(char*)"GetPlayerStringValue", GetPlayerStringValue},
	   {(char*)"GetPlayerIntValue", GetPlayerIntValue},
	   {(char*)"SetPlayerStringValue", SetPlayerStringValue},
	   {(char*)"SetPlayerIntValue", SetPlayerIntValue},
	   {NULL, NULL}
	};
	int i = 0;
	while(true) {
		CFuncTable *table = (CFuncTable *)&ctable[i];
		if(table->mpFunc == NULL) break;
		lua_register(l, table->name, table->mpFunc);
		i++;
	}
}
void processScripts(lua_State *l, LSPSnapshotMsg *ssmsg) {
	registerCFuncs(l);
	lua_getglobal(l,"ProcessResults");
	if(!lua_isfunction(l,-1)) {
		lua_pop(l,1);
		return ;
	}
	lua_pushnumber(l, ssmsg->profileid);
	lua_pushstring(l, ssmsg->data);
	lua_pushnumber(l, ssmsg->done);
	if(lua_pcall(l,3,1,0) != 0) {
		return;
	}
}
void handleSnapshot(LSPSnapshotMsg *ssmsg) {
	gameInfo *game;
	game = server.options->gameInfoIDProc(ssmsg->gameid);
	if(game == NULL) return; //shouldn't happen..
	char fpath[256];
	sprintf_s(fpath,sizeof(fpath),"files/legacystatsprocessor/lua/%s/scripts.cfg",game->name);
	if(!file_exists(fpath)) {
		return;
	}
	Config *conf = new Config(fpath);
	configVar *var = conf->getRootArray((char*)"process");
	if(var == NULL) {
		delete conf;
		return;
	}
	lua_State *l;
	int numfiles = conf->getArrayInt(var, (char*)"filecount");
	char name[32];
	for(int i=0;i<numfiles;i++) {
		l = lua_open();
		openlualibs(l);
		sprintf_s(name,sizeof(name),"file%d",i);
		sprintf_s(fpath,sizeof(fpath),"files/legacystatsprocessor/lua/%s/%s",game->name,conf->getArrayString(var,name));
		luaL_dofile(l, fpath);
		processScripts(l,ssmsg);
		lua_close(l);
	}
	delete conf;
	return;
}

