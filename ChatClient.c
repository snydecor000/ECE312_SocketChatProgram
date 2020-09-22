// Cory Snyder and Tristen Foisy
// 9/22/2020
//
// ECE312 Project 1: Socket Chat Program
//
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFFER_LEN 256  //Defines how long a message can be
#define USER_LEN 100     //Defines how long a username can be

//Global Variables 
char username[USER_LEN];
char otherUsername[USER_LEN];

void error(char *msg) {
    perror(msg);
    exit(0);
}

void * sendMessage(void * socket) {
    int sockfd, ret;
    char buffer[BUFFER_LEN];
    sockfd = (int) socket;
    int n;

    bzero(buffer,BUFFER_LEN);//Clear Buffer
    
    while(strcmp(buffer,"exit\n") != 0) {

        printf("<you>: ");
        fflush( stdout );
        bzero(buffer,BUFFER_LEN);
        fgets(buffer,BUFFER_LEN-1,stdin);

        n = write(sockfd,buffer,strlen(buffer));
        
        if (n < 0) 
            error("ERROR writing to socket");
    }

    printf("Closing connection\n");
    close(sockfd);
    exit(0);
}

void * receiveMessage(void * socket) {
    int sockfd, ret;
    char buffer[BUFFER_LEN];
    char fromUser[USER_LEN+4];
    sockfd = (int) socket;
    int n;
    char userEnd[] = ">: ";
    strcpy(fromUser, "<");
    strcat(fromUser, otherUsername);
    strcat(fromUser, ">: ");

    bzero(buffer,BUFFER_LEN);//Clear Buffer
    
    while((n = read(sockfd,buffer,BUFFER_LEN-1)) > 0) { //The code waits for this.  Fill Buffer with the message
        if (n < 0) {
            error("ERROR reading from socket");
        }
        printf("\n%s%s",fromUser,buffer);
        fflush( stdout );
        printf("<you>: ");
        fflush( stdout );
        bzero(buffer,BUFFER_LEN);//Clear Buffer
    }  

    if (n < 0) {
        printf("Error receiving data!\n");
    } else {
        printf("Closing connection\n");
        
    }
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_LEN];
    portno = 0;
    pthread_t sendThread;
    pthread_t readThread;
    int ret;

    if (argc < 2) {
        fprintf(stderr,"usage %s hostname\n", argv[0]);
        exit(0);
    }

    //Get the port number the user would like to connect to 
    while(portno == 0) {
        printf("Enter a port # to listen to: ");
        bzero(buffer, BUFFER_LEN);
        fgets(buffer, BUFFER_LEN-1, stdin);
        portno = atoi(buffer);
        if(portno < 2000 || portno > 65535) {
            portno = 0;
            printf("Please enter a valid port # from 2000 to 65535\n");
            fflush( stdout );
        }
    }

    //Get this host's username
    printf("Please enter a username: ");
    fflush( stdout );
    bzero(username, USER_LEN);
    fgets(username, USER_LEN-1, stdin);
    username[strlen(username)-1] = '\0';

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {   
        error("ERROR opening socket");
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    //Send this host's username
    n = write(sockfd, username, strlen(username));
    if (n < 0)
        error("ERROR writing to socket");

    //Get the other host's username
    n = read(sockfd, otherUsername, USER_LEN - 1);
    if (n < 0)
        error("ERROR reading from socket");

    printf("Connection established with %s (%s)\n", argv[1], otherUsername);

    //creating a new thread for sending messages to the client
    if (ret = pthread_create(&sendThread, NULL, sendMessage, (void *) sockfd)) {
        printf("ERROR: Return Code from pthread_create() is %d\n", ret);
        error("ERROR creating thread");
    }

    //creating a new thread for receiving messages from the client
    if (ret = pthread_create(&readThread, NULL, receiveMessage, (void *) sockfd)) {
        printf("ERROR: Return Code from pthread_create() is %d\n", ret);
        error("ERROR creating thread");
    }

    //
    while(1){

    }
    return 0;
}