#include "main.h"
#define PACKET_QUERY              0x00 //S -> C
#define PACKET_CHALLENGE          0x01 //S -> C
#define PACKET_ECHO               0x02 //S -> C (purpose..?)
#define PACKET_ECHO_RESPONSE      0x05  // 0x05, not 0x03 (order) | C -> S
#define PACKET_HEARTBEAT          0x03 //C -> S
#define PACKET_ADDERROR           0x04 //S -> C
#define PACKET_CLIENT_MESSAGE     0x06 //S -> C
#define PACKET_CLIENT_MESSAGE_ACK 0x07 //C -> S
#define PACKET_KEEPALIVE          0x08 //S -> C | C -> S
#define PACKET_PREQUERY_IP_VERIFY 0x09 //S -> C
#define PACKET_AVAILABLE          0x09 //C -> S
#define PACKET_CLIENT_REGISTERED  0x0A //S -> C
#define QR_PING_TIME 120
#define CHALLENGE_LEN 20
class Client {
	public:
		Client(int sd, struct sockaddr_in *peer);
		~Client();
		void handleIncoming(char *buff, int len);
		char *findServerValue(char *name);
		time_t getLastPing();
		struct sockaddr_in *getSockAddr();
		std::list<customKey *> getServerKeys();
		std::list<customKey *> getRules(); //allocates memory
		std::list<customKey *> copyServerKeys(); //allocates memory
		gameInfo *getGameInfo();
		uint16_t getPort();
		uint32_t getAddress();
		uint32_t getStateChanged();
		uint32_t getServerAddress();
		uint32_t getServerPort();
		void sendMsg(void *data, int len);
		bool isServerRegistered();
		countryRegion *getCountry();
	private:
		void handleServerData(char *buff, int len);
		void handleAvailable(char *buff, int len);
		void handleHeartbeat(char *buff, int len);
		void handleChallenge(char *buff, int len);
		void handleKeepalive(char *buff,int len);
		void handleLegacyHeartbeat(char *buff,int len);
		void handleLegacyIncoming(char *buff, int len);
		void setData(char *variable, char *value);
		void pushServer();
		void pushDelete();
		void clearKeys();
		bool isTeamString(char *string);
		customKey *findKey(char *name);
		time_t connecttime;
		time_t lastPing;
		uint8_t instancekey[REQUEST_KEY_LEN]; //used by gamespy to prevent UDP packet spoofing
		char challenge[CHALLENGE_LEN + 1];
		int sd;
		bool sentChallenge;
		bool hasInstanceKey;
		bool serverRegistered;
		bool legacyQuery;
		countryRegion *country;
		gameInfo *game;
		//server info
		std::list<customKey *> serverKeys;
		std::list<indexedKey *> playerKeys;
		std::list<indexedKey *> teamKeys;
		//end
		struct sockaddr_in sockinfo;
		struct sockaddr_in servinfo;
};
