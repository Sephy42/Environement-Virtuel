extern "C" {

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
}
#include <iostream>
int main(int argc, char **argv) 
{  
	int		sock, n;
	struct addrinfo	hints, *res, *ressave;
	int longueurAdr;
	struct sockaddr *adrDest;

	// Récupération de l'adresse de destination (@IP+port)
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; // le système choisira IPv4 ou IPv6
	hints.ai_socktype = SOCK_DGRAM; // on veut UDP
	// à la place de localhost on peut mettre le nom de la machine
	// ou son adresse IP (v4 ou v6)
    if ( (n = getaddrinfo("localhost", "8123", &hints, &res)) != 0) {
		printf("erreur getaddrinfo : %s\n", gai_strerror(n));
		return 1;
	}
	ressave = res;

	do { // Construction d'un socket compatible avec cette adresse
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock >= 0)
			break;		// ça a marché
	} while ( (res = res->ai_next) != NULL);

	// aucune adresse n'a fonctionné, errno est positionné par socket()
	if (res == NULL) {
		perror("erreur socket");
		return 1;
	}

	// on alloue puis on recopie l'adresse IP + port de destination
	adrDest = (struct sockaddr*)malloc(res->ai_addrlen);
	memcpy(adrDest, res->ai_addr, res->ai_addrlen);
	longueurAdr = res->ai_addrlen;
	// on libère la mémoire allouée par getaddrinfo
	freeaddrinfo(ressave);
	
	#define BUFFERLEN 256                            
	char buf[BUFFERLEN];        // Tampon pour le message
    sprintf(buf, "Ibonjour !");  // On écrit le message dans le tampon

    while (1){

        // Envoie le message
        if (sendto(sock, buf, strlen(buf) + 1, 0, adrDest, longueurAdr) <0) {
              perror("sendto");
              return 1;
        }


        std::cout<<"envoie"<<std::endl;




            /* envoyer un message */

        //à deplacer dans tank.cpp world::update à la fin
        fd_set ens;
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 50000; //attendre au max pendant 50ms = 50000us

        FD_ZERO(&ens);
        FD_SET(sock,&ens);

        while (select(sock+1, &ens, NULL, NULL, &timeout)>0){ //tous les timeout on verifie qu'il y ai rien de new dans ens (ui est le sock bah ouai)

            //recevoir message et traiter = creer nouveau tank, mettre à jour => map de tank, après on accedera à l'id du tank pour mettre à jour position etc
           int cnt;
           char message[1024];

           // on recupere le message
           cnt = recv(sock, message, sizeof(message),0);
           if (cnt < 0) {
                perror("erreur de recv");
                exit(1);
           }
           // on traite le message ici

           std::cout<<message<<std::endl;

        }

    }
    return 0;
 }


