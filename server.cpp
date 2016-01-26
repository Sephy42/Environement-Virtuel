#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "server.h"

/*
 *  server.h IPv4 version
 *
 *  Created by Patrice Torguet on 14/01/08.
 *
 */

#define EXAMPLE_PORT	8123

Server::Server() {
	sock = 0;
	nextId = 0;
}
		
int Server::init() {
	  struct sockaddr_in addr;
	  
      sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock < 0) {
          perror("socket");
          return -1;
      }

      bzero(&addr, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_port = htons(EXAMPLE_PORT);

	  if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
              perror("bind");
              return -2;
	  }
	  
	  return 1;
}
		
int Server::manageUpdate(char buf[], int length) {
	for(set<struct sockaddr_in>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (sendto(sock, buf, length, 0, (struct sockaddr*)&(*it), sizeof(struct sockaddr_in))<0) {
			perror("sendto");
			return -1;
		}
	}
	return 1;
}
		
int Server::mainLoop() {
	char buf[512];
	struct sockaddr_in addr;
	socklen_t addrlen;
	int received;

	while(1) {
		addrlen = sizeof(addr);
		if((received = recvfrom(sock, buf, 512, 0, (struct sockaddr*)&addr, &addrlen))<0) {
			perror("recvfrom");
			return -1;
		}
		//printf("recu %d octets de %s,%d\n", received, inet_ntoa(addr.sin_addr), addr.sin_port);
		// nouveau client ?
		if(clients.find(addr) == clients.end()) {
			clients.insert(addr);
			// doit-on lui envoyer un ID ?
			if(buf[0]=='I') {
				memcpy(buf, &nextId, sizeof(int));
				printf("nouveau client d'id %d\n", nextId);
				nextId++;
				// on lui envoie son ID
				if (sendto(sock, buf, sizeof(int), 0, (struct sockaddr*)&addr, sizeof(addr))<0) {
					perror("sendto");
					return -1;
				}

                continue;
			}
		}
		if(buf[0]!='I') {
			// on diffuse a tous
			manageUpdate(buf, received);
		}
	}
}

int main() {
	Server s;
	if(s.init())
		if (s.mainLoop())
			return 0;
		else
			return 2;
	else
		return 1;
}
