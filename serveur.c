#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>



int sockSend,sockReceive;

void Connection(){
	
	struct sockaddr_in serverSend,serverReceive;
	
	//Create socket
	sockSend = socket(AF_INET , SOCK_STREAM , 0);
	if (sockSend == -1)
	{
		printf("Could not create socketSend");
	}
	puts("SocketSend created");
	
	serverSend.sin_addr.s_addr = inet_addr("192.168.2.60");
	serverSend.sin_family = AF_INET;
	serverSend.sin_port = htons( 8889 );

	//Connect to remote server
	if (connect(sockSend , (struct sockaddr *)&serverSend , sizeof(serverSend)) < 0)
	{
		perror("connect failed to Send. Error");
		exit(1);
	}
	
	puts("Connected to Send\n");
	
	sockReceive = socket(AF_INET , SOCK_STREAM , 0);
	if (sockReceive == -1)
	{
		printf("Could not create socketReceive");
	}
	puts("SocketReceive created");
	
	serverReceive.sin_addr.s_addr = inet_addr("192.168.2.60");
	serverReceive.sin_family = AF_INET;
	serverReceive.sin_port = htons( 8888 );

	//Connect to remote server
	if (connect(sockReceive , (struct sockaddr *)&serverReceive , sizeof(serverReceive)) < 0)
	{
		perror("connect failed to Receive. Error");
		exit(1);
	}
	
	puts("Connected to Receive\n");
}

void Sendmessage(char* format){
		char message[25];
		//Send some data
		sprintf(message, "%s\n", format);

		if( send(sockSend , message , strlen(message) , 0) < 0)
		{
			puts("Send failed");
			exit(1);
		}
}
void Receivemessage()
{
	char server_reply[20];
	if( recv(sockReceive , server_reply , 20 , 0) < 0)
		{
			puts("recv failed");
			exit(1);
		}
	puts("Server reply :");
	puts(server_reply);
	
}

void Deconnection(){
	close(sockSend);
	close(sockReceive);
}