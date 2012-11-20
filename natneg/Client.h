#ifndef NN_CLIENT_H
#define NN_CLIENT_H
#include "main.h"
#include "structs.h"
class Client {
	public:
		Client(int sd, struct sockaddr_in *peer, int instance);
		~Client();
		void handleIncoming(char *buff, int len);
		struct sockaddr_in *getSockAddr();
		gameInfo *getGameInfo();
		uint16_t getPort();
		uint32_t getAddress();
		int getSocket();
		int getCookie();
		int getInstance();
		int getIndex();
		time_t getConnectTime();
		time_t getLastPacket();
		time_t getSendConnectTime();
		bool getConnected();
		bool getConnectedAck();
		void sendDeadBeatNotice();
		void trySendConnect(bool sendToOther = true);
	private:
		void handleInitPacket(NatNegPacket *packet);
		void handleAddressCheck(NatNegPacket *packet);
		void handleNatifyRequest(NatNegPacket *packet);
		void handleReport(NatNegPacket *packet);
		void SendERTReply(char type, NatNegPacket *packet);
		void SendConnectPacket(Client *user, bool sendToOther = true);
		int instance;
		uint8_t version;
		time_t connecttime;
		time_t lastPacket;
		time_t sentconnecttime;
		int sd;
		int cookie;
		char cindex;
		bool connected;
		bool gotInit;
		bool gotConnectAck;
		gameInfo *game;
		struct sockaddr_in sockinfo;
};
#endif
