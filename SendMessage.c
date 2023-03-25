#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void SendMessage(char *message) {
    int sockfd;
    struct sockaddr_in dest_addr;

    // Create a TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return;
    }

    // Set the destination address and port
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(1234); // Replace with the actual port number
    inet_pton(AF_INET, "192.168.1.100", &dest_addr.sin_addr); // Replace with the actual IP address of the Raspberry Pi

    // Connect to the destination
    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1) {
        perror("connect");
        close(sockfd);
        return;
    }

    // Send the message
    if (send(sockfd, message, strlen(message), 0) == -1) {
        perror("send");
    }

    // Close the socket
    close(sockfd);
}
