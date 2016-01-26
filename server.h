/*
 *  server.h IPv4 version
 *
 *  Created by Patrice Torguet on 14/01/08.
 *
 */
#ifndef SERVER_H
#define SERVER_H
 
#include <netinet/in.h>

#include <set>
#include <functional>

using namespace std;

struct less_addr : public binary_function<struct sockaddr_in, struct sockaddr_in, bool> {
	bool operator()(const struct sockaddr_in i1, const struct sockaddr_in i2) {
		if (i1.sin_addr.s_addr < i2.sin_addr.s_addr)
			return 1;
		if ((i1.sin_addr.s_addr == i2.sin_addr.s_addr) &&
			(i1.sin_port < i2.sin_port))
			return 1;
		return 0;
	}
};
	
class Server {
	private:
		int sock;
		set<struct sockaddr_in, less_addr> clients;
		int nextId;
	public:
		Server();
		
		int init();
		
		int manageUpdate(char buf[], int length);
		
		int mainLoop();
};

#endif