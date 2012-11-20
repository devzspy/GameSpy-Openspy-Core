#ifndef _NSPHandler
#define _NSPHandler
#include "main.h"
#include "structs.h"
extern "C" {
	static void openlualibs(lua_State *l);
	void registerCFuncs();
	void handleSnapshot(LSPSnapshotMsg *ssmsg);
}
#endif
