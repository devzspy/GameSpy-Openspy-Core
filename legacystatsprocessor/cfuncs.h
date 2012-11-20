#include "main.h"
#include "structs.h"
extern "C" {
	int chctest(lua_State *L);
	int GetSSInt(lua_State *L);
	int GetSSString(lua_State *L);
	int GetPlayerIntValue(lua_State *L);
	int GetPlayerStringValue(lua_State *L);
	int SetPlayerIntValue(lua_State *L);
	int SetPlayerStringValue(lua_State *L);
}
