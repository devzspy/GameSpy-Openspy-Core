#ifndef _INC_LOOKUP
#define _INC_LOOKUP
#include "main.h"
#include <common/helpers.h>
#include <common/mysql_helpers.h>
#include <playerspy/gp.h>
void sendError(int sd, char *msg);
void checkEmailValid(int sd,char *buff);
void sendNicks(int sd, char *buff);
void checkNick(int sd, char *buff);
void newUser(int sd, char *buff);
void searchUsers(int sd, char *buff);
void sendReverseBuddies(int sd,char *msg);
#endif
