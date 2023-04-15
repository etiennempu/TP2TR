#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define SERV_PORT 1231

#define TAILLE_MESSAGE 22 // Longueur maximum d'un message 7 (LG + identifiant : 1|2|3 + valeur : 00...100 + \n) * 3 messages + \0

struct sockaddr_in serverReceive;
int sockReceive;

void OuvrirServeur(){
	
	// Créer le socket du serveur
	if ((sockReceive = socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{
		perror("Could not create socketReceive");
		exit(1);
	}
	puts("SocketReceive created");
	
	// Lier l'adresse locale au socket
	memset(&serverReceive, 0, sizeof(serverReceive));
	serverReceive.sin_addr.s_addr = htonl(INADDR_ANY);
	serverReceive.sin_family = AF_INET;
	serverReceive.sin_port = htons( SERV_PORT );
	
}

void AttenteOuvertureServeur() {
	while ((connect(sockReceive , (struct sockaddr *)&serverReceive , sizeof(serverReceive))) == -1) ;
}

int test_close() {
	char* message = "1";
	int err = send(sockReceive , message , strlen(message) , 0);
	return err;
}

char* ReceiveMessage()
{
	char* buffer = calloc(TAILLE_MESSAGE, sizeof(char));
	if( (recv(sockReceive , buffer , TAILLE_MESSAGE , MSG_DONTWAIT) < 0) && (errno != EAGAIN))
	{
		perror("recv failed");
		exit(1);
	}

	return buffer;
}

void FermerServeur(){
	close(sockReceive);
}