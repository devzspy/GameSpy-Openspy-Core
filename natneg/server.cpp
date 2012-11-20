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
Client *find_user(struct sockaddr_in *peer, int instance) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	struct sockaddr_in *userpeer;
	while(iterator != server.client_list.end()) {
		user=*iterator;
		userpeer = user->getSockAddr();
		if((userpeer->sin_addr.s_addr == peer->sin_addr.s_addr) && (userpeer->sin_port == peer->sin_port) && (instance == 0 || user->getInstance() == instance)) {
			return user;
		}
		iterator++;
	}
	return NULL;
}

Client *find_user(uint32_t ip, uint16_t port, int instance) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	while(iterator != server.client_list.end()) {
		user=*iterator;
		if(user->getAddress() == ip && user->getPort() == port && (instance == 0 || user->getInstance() == instance)) {
			return user;
		}
		iterator++;
	}
	return NULL;
}
Client *find_user_by_cookie(int cookie, int instance) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	while(iterator != server.client_list.end()) {
		user=*iterator;
		if(user->getCookie() == cookie && (user->getInstance() == instance)) {
			return user;
		}
		iterator++;
	}
	return NULL;
}
Client *find_user_by_cookie_index(int cookie, int instance, int index) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	while(iterator != server.client_list.end()) {
		user=*iterator;
		if(user->getCookie() == cookie && (user->getInstance() == instance) && user->getIndex() == index) {
			return user;
		}
		iterator++;
	}
	return NULL;
}
