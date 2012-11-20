#include "main.h"
#include "structs.h"
#include "handler.h"
#define DISCONNECT_TIME 300
legacyStatsProcessor server;
modInfo moduleInfo = {"legacystatsprocessor","Data handler for legacy stats"};
modInfo *openspy_modInfo() {
	return &moduleInfo;
}
void *openspy_mod_run(modLoadOptions *options) {
    MYSQL_RES *res;
    MYSQL_ROW row;
    server.conn = mysql_init(NULL);
    server.options = options;
    int on_a = 1;
    mysql_options(server.conn,MYSQL_OPT_RECONNECT, (char *)&on_a);
   /* Connect to database */
   if (!mysql_real_connect(server.conn, options->mysql_server,
         options->mysql_user, options->mysql_password, options->mysql_database, 0, NULL, CLIENT_MULTI_RESULTS)) {
      fprintf(stderr, "%s\n", mysql_error(server.conn));
      return NULL;
   }
    for(;;) {
	//every 5 mins or so check all scripts to see if they need to be executed
	sleep(100000);

    }
      return NULL;
}
bool openspy_mod_query(char *sendmodule, void *data,int len) {
	LSPBaseMsg *spmsg = (LSPBaseMsg *)data;
	LSPSnapshotMsg *ssmsg;
	switch(spmsg->msgID) {
		case ELSPMsgID_ProcessSnapshot:
			ssmsg = (LSPSnapshotMsg *)spmsg->data;
			handleSnapshot(ssmsg);			
		break;
	}
	return false;
}
