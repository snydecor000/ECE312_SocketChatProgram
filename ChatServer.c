//Cory Snyder and Tristen Foisy

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_LEN 256 //Defines how long a message can be

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, clilen;
    int portno = 0;
    char buffer[BUFFER_LEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    while(portno == 0) {
        printf("Enter a port # to listen to: ");
        bzero(buffer, BUFFER_LEN);
        fgets(buffer, BUFFER_LEN-1, stdin);
        portno = atoi(buffer);
        if(portno < 2000 || portno > 65535) {
            portno = 0;
            printf("Please enter a valid port # from 2000 to 65535\n");
        }
    }

    //Attempt to open the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //If there was an error opening the socket
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    //Clear the server address, and put all the neccesary info in it
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //Attempt to bind with the socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }
    bzero(buffer,BUFFER_LEN);//Clear Buffer
    n = read(newsockfd,buffer,BUFFER_LEN-1);//Fill Buffer with the message
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);
    n = write(newsockfd,"I got your message",18);
    if (n < 0) error("ERROR writing to socket");
    return 0; 
}