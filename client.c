#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define CLIENT_PORT 1232

#define TAILLE_MESSAGE 10

struct sockaddr_in serverSend;
int sockSend;

void OuvrirClient() {

	// Créer le socket d'écoute du client
	if ((sockSend = socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{
		perror("Could not create socketSend");
		exit(1);
	}
	puts("SocketSend created");

	// Lier l'adresse locale au socket
	memset(&serverSend, 0, sizeof(serverSend));
	serverSend.sin_addr.s_addr = htonl(INADDR_ANY);;
	serverSend.sin_family = AF_INET;
	serverSend.sin_port = htons( CLIENT_PORT );

}

void AttenteOuvertureClient() {
	while ((connect(sockSend , (struct sockaddr *)&serverSend , sizeof(serverSend))) == -1) ;
}

void SendMessage(char* format){
		char message[TAILLE_MESSAGE];
		//Send some data
		sprintf(message, "%s\n", format);

		if( send(sockSend , message , strlen(message) , 0) < 0)
		{
			puts("Send failed");
			exit(1);
		}
}

void FermerClient(){
	close(sockSend);
}