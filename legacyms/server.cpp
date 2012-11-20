#include "server.h"
#include "Client.h"
extern serverInfo server;
void deleteClient(Client *client) {
	std::list<Client *>::iterator iterator;
	iterator=server.client_list.begin();
	while(iterator != server.client_list.end()) {
		if(*iterator==client) {
			iterator = server.client_list.erase(iterator);
			delete client;
		} else
		iterator++;

	}
}
Client *find_user(struct sockaddr_in *peer) {
	std::list<Client *>::iterator iterator;
	Client *user;
	struct sockaddr_in *userpeer;
	iterator = server.client_list.begin();
	while(iterator != server.client_list.end()) {
		user=*iterator;
		userpeer = user->getSockAddr();
		if((userpeer->sin_addr.s_addr == peer->sin_addr.s_addr) && (userpeer->sin_port == peer->sin_port)) {
			return user;
		}
		iterator++;
	}
	return NULL;
}
Client *find_user(uint32_t ip, uint16_t port) {
	struct sockaddr_in peer;
	peer.sin_addr.s_addr = ip;
	peer.sin_port = port;
	return find_user(&peer);
}
