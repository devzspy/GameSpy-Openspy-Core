#include <common/helpers.h>
#include "cfuncs.h"
#include "statsmanage.h"
int chctest(lua_State *L) {
	lua_newtable(L);
	lua_pushnumber(L,1);
	lua_pushstring(L,"teststring");
	lua_settable(L,-3);
	lua_pushnumber(L,2);
	lua_pushstring(L,"teststring2");
	lua_settable(L,-3);
	return 1;
}
int GetSSInt(lua_State *L) {
//GetSSInt(data, value)
	const char *data = lua_tostring(L, 1);
	const char *value = lua_tostring(L, 2);
	if(data == NULL || value == NULL) return -1;
	int ret = find_paramint((char *)value,(char *)data);
	lua_pushnumber(L, ret);
	return 1;
}
int GetSSString(lua_State *L) {
	const char *data = lua_tostring(L, 1);
	const char *value = lua_tostring(L, 2);
	if(data == NULL || value == NULL) return -1;
	char dst[256];
	if(!find_param((char *)value,(char *)data,(char *)&dst,sizeof(dst))) {
		return -1;
	}
	lua_pushstring(L, dst);
	return 1;
}
int GetPlayerIntValue(lua_State *L) {
	int gameid = lua_tonumber(L, 1);
	persisttype_t type = (persisttype_t )(int)lua_tonumber(L, 2);
	int index = lua_tonumber(L, 3);
	int profileid = lua_tonumber(L, 4);
	const char *name = lua_tostring(L, 5);
	int key = getKeyInt(gameid,type,index,profileid,(char *)name);
	lua_pushnumber(L, key);
	return 1;
}
int GetPlayerStringValue(lua_State *L) {
	char data[256];
	int gameid = lua_tonumber(L, 1);
	persisttype_t type = (persisttype_t )(int)lua_tonumber(L, 2);
	int index = lua_tonumber(L, 3);
	int profileid = lua_tonumber(L, 4);
	const char *name = lua_tostring(L, 5);
	getKeyString(gameid,type,index,profileid,(char *)name,data,sizeof(data));
	lua_pushstring(L,data);
	return 1;
}
int SetPlayerIntValue(lua_State *L) {
	int gameid = lua_tonumber(L, 1);
	persisttype_t type = (persisttype_t )(int)lua_tonumber(L, 2);
	int index = lua_tonumber(L, 3);
	int profileid = lua_tonumber(L, 4);
	const char *name = lua_tostring(L, 5);
	int value = lua_tonumber(L, 6);
	setKeyInt(gameid,type,index,profileid,(char *)name,value);
	return 1;
}
int SetPlayerStringValue(lua_State *L) {
	int gameid = lua_tonumber(L, 1);
	persisttype_t type = (persisttype_t )(int)lua_tonumber(L, 2);
	int index = lua_tonumber(L, 3);
	int profileid = lua_tonumber(L, 4);
	const char *name = lua_tostring(L, 5);
	const char *value = lua_tostring(L, 6);
	setKeyString(gameid,type,index,profileid,(char *)name,(char *)value);
	return 1;
}
