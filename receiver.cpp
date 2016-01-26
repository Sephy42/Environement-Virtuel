extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/socket.h>
#include <netdb.h>
}

int main(int argc, char **argv) 
{  
	int	   sock, n;
	struct addrinfo	hints, *res, *ressave;
	socklen_t longueurAdr;
	char machine[NI_MAXHOST];
	char service[NI_MAXSERV];

	// Récupération de l'adresse de liaison (@IP Any + port choisi)
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE; // on est serveur ici
	hints.ai_family = AF_UNSPEC; // le système choisit IPv4 ou IPv6
	hints.ai_socktype = SOCK_DGRAM; // on veut UDP
	// NULL correspond à l'adresse Any (0.0.0.0 pour IPv4 et 0::0 pour IPv6)
	if ( (n = getaddrinfo(NULL, "13214", &hints, &res)) != 0) {
		printf("erreur getaddrinfo : %s\n", gai_strerror(n));
		return 1;
	}
	ressave = res;

	do { // on essaie de construire un socket compatible
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock < 0)
			continue;		// ça n'a pas marché on passe à la suivante
		// on essaie de se lier au port
		if (bind(sock, res->ai_addr, res->ai_addrlen) == 0)
			break;			// ça a marché

		close(sock);		// erreur de bind, on ferme le socket
	} while ( (res = res->ai_next) != NULL); // passe à la suivante

	// aucune adresse n'a fonctionné, on s'arrête
	if (res == NULL) {
		perror("erreur bind ou socket");
		return 1;
	}

	// conserve la longueur de l'adresse pour les recvfrom
	longueurAdr = res->ai_addrlen;
	printf("longueur adr : %d\n", longueurAdr);

	// on libère la mémoire
	freeaddrinfo(ressave);
		
	#define BUFFERLEN 256
	char buf[BUFFERLEN]; // Tampon pour recevoir le message

	struct sockaddr *srcAdr; // Contiendra l'@IP et le port de l'émetteur
	srcAdr = (struct sockaddr*) malloc(longueurAdr);
	bzero((char *)srcAdr, longueurAdr);  // init. à 0

	// Attends et reçoit les données 
	if (recvfrom(sock, buf, sizeof(buf), 0, srcAdr, &longueurAdr) < 0) {
      	perror("recvfrom");
      	return 1;
	} 
	
	// on essaie de récupérer l'adresse IP et le port de l'émetteur
	if((n = getnameinfo(srcAdr, longueurAdr, machine, NI_MAXHOST, 
	                  service, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV)) == 0)
		printf("recu : %s depuis %s:%s\n",buf, machine, service);
	else {
		printf("recu : %s\n",buf);
		printf("erreur getnameinfo : %s\n", gai_strerror(n));
	}
    return 0;
 }
