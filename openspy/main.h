#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <common/helpers.h>
#include "module.h"
#ifdef _WIN32
#define MODULE_EXTENSION ".dll"
#define sleep Sleep
#include <WinSock2.h>
#else
#define MODULE_EXTENSION ".so"
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#endif
#include "structs.h"
#include <signal.h>
#include <mysql/mysql.h>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
extern char *mysql_server;
extern char *mysql_password;
extern char *mysql_user;
extern char *mysql_database_matrix;
extern gameInfo *games;
