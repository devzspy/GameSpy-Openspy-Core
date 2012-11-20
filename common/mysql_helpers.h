#include "helpers.h"
#include <mysql/mysql.h>
#include <playerspy/gp.h>
void clearResults(MYSQL *sql);
int getUserIDFromEmail(MYSQL *sql, char *email);
int getProfileUserID(MYSQL *sql, int profileid);
bool tryPassword(MYSQL *sql, int userid, char *pass);
bool tryPasswordMD5(MYSQL *sql, int userid, char *pass);
int getProfileIDFromNickEmail(MYSQL *sql, char *nick, char *email);
int makeNewProfile(MYSQL *sql, char *nick, int userid);
bool isUserIDEmailHidden(MYSQL *sql, int userid);
void getUserIDPass(MYSQL *sql, int userid, char *dst, int dstlen);
int getProfileIDFromUniquenick(MYSQL *sql, char *uniquenick);
void getProfileIDPass(MYSQL *sql, int profileid, char *dst, int dstlen);
int makeNewProfileWithUniquenick(MYSQL *sql, char *nick, char *uniquenick, int userid);
bool validProfileID(MYSQL *sql, int profileid);
int registerUser(MYSQL *sql,char *email,char *pass);
bool getProfileInfo(MYSQL *sql,int profileid,GPIInfoCache *info);
void updateUserProfile(MYSQL *sql, GPIInfoCache *info);
uint32_t getPublicMask(MYSQL *sql, int userid);
void addAuthCdKey(MYSQL *sql, int profileid, char *cdkey, gameInfo *game);
