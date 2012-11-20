#ifndef _SB_STRUCTS_INC
#define _SB_STRUCTS_INC
#include "main.h"
#include <qr/structs.h>
//master server query port
#define MSPORT2 28910

//number of master servers
#define NUM_MASTER_SERVERS 20

//max length of field list to master server
#define MAX_FIELD_LIST_LEN 256

//max number of values in a popular value list
#define MAX_POPULAR_VALUES 255

//max number of maps to track in a map loop
#define MAX_MAPLOOP_LENGTH 16

// max number of bytes that can be received from a single recvfrom call
// This must not be higher than 2048 for PS2 insock compatability
#define MAX_RECVFROM_SIZE 2048

//states for SBServer->state 
#define STATE_BASICKEYS			(1 << 0)
#define STATE_FULLKEYS			(1 << 1)
#define STATE_PENDINGBASICQUERY	(1 << 2)
#define STATE_PENDINGFULLQUERY	(1 << 3)
#define STATE_QUERYFAILED		(1 << 4)
#define STATE_PENDINGICMPQUERY	(1 << 5)
#define STATE_VALIDPING	        (1 << 6)
#define STATE_PENDINGQUERYCHALLENGE (1 << 7)

//how long before a server query times out
#define MAX_QUERY_MSEC 2500

//game server flags
#define UNSOLICITED_UDP_FLAG	1
#define PRIVATE_IP_FLAG			2
#define CONNECT_NEGOTIATE_FLAG	4
#define ICMP_IP_FLAG			8
#define NONSTANDARD_PORT_FLAG	16
#define NONSTANDARD_PRIVATE_PORT_FLAG	32
#define HAS_KEYS_FLAG					64
#define HAS_FULL_RULES_FLAG				128

//backend query flags (set in hbmaster, don't change)
#define QR2_USE_QUERY_CHALLENGE 128

//key types for the key type list
#define KEYTYPE_STRING	0
#define KEYTYPE_BYTE	1
#define KEYTYPE_SHORT	2

//how long to make the outgoing challenge
#define LIST_CHALLENGE_LEN 8

//protocol versions
#define LIST_PROTOCOL_VERSION 1
#define LIST_ENCODING_VERSION 3

//message types for outgoing requests
#define SERVER_LIST_REQUEST		0
#define SERVER_INFO_REQUEST		1
#define SEND_MESSAGE_REQUEST	2
#define KEEPALIVE_REPLY			3
#define MAPLOOP_REQUEST			4
#define PLAYERSEARCH_REQUEST	5

//message types for incoming requests
#define PUSH_KEYS_MESSAGE		1
#define PUSH_SERVER_MESSAGE		2
#define KEEPALIVE_MESSAGE		3
#define DELETE_SERVER_MESSAGE	4
#define MAPLOOP_MESSAGE			5
#define PLAYERSEARCH_MESSAGE	6

//server list update options
#define SEND_FIELDS_FOR_ALL		1
#define NO_SERVER_LIST			2
#define PUSH_UPDATES			4
#define SEND_GROUPS				32
#define NO_LIST_CACHE			64
#define LIMIT_RESULT_COUNT		128

//player search options
#define SEARCH_ALL_GAMES		1
#define SEARCH_LEFT_SUBSTRING	2
#define SEARCH_RIGHT_SUBSTRING	4
#define SEARCH_ANY_SUBSTRING	8

//max number of keys for the basic key list
#define MAX_QUERY_KEYS 40

//how long to search on the LAN
#define SL_LAN_SEARCH_TIME 2000

//MAGIC bytes for the QR2 queries
#define QR2_MAGIC_1 0xFE
#define QR2_MAGIC_2 0xFD

//magic bytes for nat negotiation message
#define NATNEG_MAGIC_LEN 6
#define NN_MAGIC_0 0xFD
#define NN_MAGIC_1 0xFC
#define NN_MAGIC_2 0x1E
#define NN_MAGIC_3 0x66
#define NN_MAGIC_4 0x6A
#define NN_MAGIC_5 0xB2


//query types
#define QTYPE_BASIC 0
#define QTYPE_FULL  1
#define QTYPE_ICMP  2

//query strings for old-style servers
#define BASIC_GOA_QUERY "\\basic\\\\info\\"
#define BASIC_GOA_QUERY_LEN 13
#define FULL_GOA_QUERY "\\status\\"
#define FULL_GOA_QUERY_LEN 8

//maximum length of a sortkey string
#define SORTKEY_LENGTH 255

//include ICMP support by default
#ifndef SB_NO_ICMP_SUPPORT
	#undef SB_ICMP_SUPPORT
	#define SB_ICMP_SUPPORT
#endif

	//Maximum length for the SQL filter string
#define MAX_FILTER_LEN 511

	//Version defines for query protocol
#define QVERSION_GOA 0
#define QVERSION_QR2 1

#define SERVER_GROWBY 100

//for the master server info
#define INCOMING_BUFFER_SIZE 1024

#define MAX_OUTGOING_REQUEST_SIZE (MAX_FIELD_LIST_LEN + MAX_FILTER_LEN + 255)

#define ALTERNATE_SOURCE_IP 8
enum {
	ESBMsgID_PushServer = 0x20,
	ESBMsgID_DeleteServer,
};
typedef struct {
	char msgID;
	void *data;
} sbServerMsg;
typedef struct {
	uint32_t ipaddr;
	uint16_t port;
	gameInfo *game;
	countryRegion *country;
	std::list<customKey *> keys;
} sbPushMsg;
#endif
