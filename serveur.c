#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define SERV_PORT 1231

#define TAILLE_MESSAGE 10

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
	serverReceive.sin_addr.s_addr = inet_addr("192.168.2.60");
	serverReceive.sin_family = AF_INET;
	serverReceive.sin_port = htons( SERV_PORT );

	//Connect to remote server
	if ((connect(sockReceive , (struct sockaddr *)&serverReceive , sizeof(serverReceive))) != 0)
	{
		perror("connect failed to Receive. Error");
		exit(1);
	}
	puts("Connected to Receive\n");
}

char* ReceiveMessage()
{
	char* buffer = malloc(TAILLE_MESSAGE);
	if( recv(sockReceive , buffer , TAILLE_MESSAGE , 0) < 0)
	{
		perror("recv failed");
		exit(1);
	}	
	return buffer;
}

void FermerServeur(){
	close(sockReceive);
}