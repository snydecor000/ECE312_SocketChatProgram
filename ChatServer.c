// Cory Snyder and Tristen Foisy
// 9/22/2020
//
// ECE312 Project 1: Socket Chat Program
// ChatServer.c
//
// ChatServer is designed to take a port number as an input in order to 
// create a socket for clients to attach to. It allows the concurrent 
// sending and receiving of messages which is implemented via two seperate 
// pthreads. 
// 
// The general flow of the program is as follows:
// 1. Asks user for the port number 
// 2. Asks user for their username
// 3. Creates a socket and binds with it
// 4. Listens and waits to accept a client connection
// 5. Exchanges usernames with the client
// 6. Prints confirmation of the connection
// 7. Starts two threads, one for sending messages, one for receiving
// 8. Waits for "exit" to be sent or received before terminating

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_LEN 256  //Defines how long a message can be
#define USER_LEN 100     //Defines how long a username can be

char username[USER_LEN]; // stores the username associated with this computer
char otherUsername[USER_LEN]; //stores the username associated with the client's computer

void error(char *msg) {
    perror(msg);
    exit(1);
}

// A method, established in a thread, that receives messages from the client. It loops 
// forever and allows constant reception of messages. 
void* receiveMessage(void* socket) {
    // Define local variables
    int sockfd, ret;
    char buffer[BUFFER_LEN];
    char fromUser[USER_LEN+4];
    sockfd = (int)socket;
    int n;

    // builds a String that displays the other user's username
    // '<username>: '
    char userEnd[] = ">: ";
    strcpy(fromUser, "<");
    strcat(fromUser, otherUsername);
    strcat(fromUser, ">: ");

    bzero(buffer, BUFFER_LEN);//Clear Buffer

    //while there are no read errors, keep waiting to receive messages
    while ((n = read(sockfd, buffer, BUFFER_LEN - 1)) > 0) {
        if (n < 0) {
            error("ERROR reading from socket");
        }
        printf("\n%s%s", fromUser, buffer);
        fflush( stdout );
        printf("<you>: ");
        fflush( stdout );
        bzero(buffer, BUFFER_LEN);//Clear Buffer
    }

    if (n < 0)
        printf("Error receiving data!\n");
    else
        printf("Closing connection\n"); 
        
    close(sockfd);
    exit(0);
}

// A method, established in a thread, that sends messages to the client. It loops 
// forever and allows constant input of messages, sending them when enter is pressed. 
// If "exit" is typed and sent, the method ends and the connection is closed. 
void* sendMessage(void* socket) {
    // Define local variables
    int sockfd, ret;
    char buffer[BUFFER_LEN];
    sockfd = (int)socket;
    int n;

    bzero(buffer, BUFFER_LEN);//Clear Buffer

    //while the message being sent isn't "exit," keep the ability to send messages
    while (strcmp(buffer, "exit\n") != 0) {

        printf("<you>: ");
        fflush( stdout );
        bzero(buffer, BUFFER_LEN);
        fgets(buffer, BUFFER_LEN - 1, stdin);

        n = write(sockfd, buffer, strlen(buffer));

        if (n < 0)
            error("ERROR writing to socket");
    }

    //Close the connection if "exit" is sent
    printf("Closing connection\n");
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    // Declare local variables
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUFFER_LEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    pthread_t readThread;
    pthread_t sendThread;
    int ret;

    portno = 0;
    clilen = sizeof(cli_addr);

    //Get the port number the user would like to connect to
    while(portno == 0) {
        printf("Enter a port # to listen to: ");
        bzero(buffer, BUFFER_LEN);// Clears Buffer
        fgets(buffer, BUFFER_LEN-1, stdin);
        portno = atoi(buffer);
        // check validity of port specified
        if(portno < 2000 || portno > 65535) {
            portno = 0;
            printf("Please enter a valid port # from 2000 to 65535\n");
            fflush( stdout );
        }
    }

    //Get this host's username from the user
    printf("Please enter a username: ");
    fflush( stdout );
    bzero(username, USER_LEN);
    fgets(username, USER_LEN-1, stdin);
    username[strlen(username) - 1] = '\0';
    printf("Waiting for connection...\n");
    fflush( stdout );

    //Attempt to open the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    //Clear the server address, and put all the neccesary info in it
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //Attempt to bind with the socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    //Listen for the client
    listen(sockfd,1);

    //Accept the connection to the client, wait here till connection
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);//The code waits for this to happen
    if (newsockfd < 0)
        error("ERROR on accept");

    //Send this host's username
    n = write(newsockfd, username, strlen(username));
    if (n < 0)
        error("ERROR writing to socket");

    //Get the other host's username
    n = read(newsockfd, otherUsername, USER_LEN - 1);
    if (n < 0)
        error("ERROR reading from socket");
    
    //Print confirmation of the connection
    printf("Connection established with %s (%s)\n",inet_ntoa(cli_addr.sin_addr),otherUsername);

    //creating a new thread for receiving messages from the client
    if (ret = pthread_create(&readThread, NULL, receiveMessage, (void *) newsockfd)) {
        printf("ERROR: Return Code from pthread_create() is %d\n", ret);
        error("ERROR creating thread");
    }

    //creating a new thread for sending messages to the client
    if (ret = pthread_create(&sendThread, NULL, sendMessage, (void*)newsockfd)) {
        printf("ERROR: Return Code from pthread_create() is %d\n", ret);
        error("ERROR creating thread");
    }

    //Have this thread loop forever and wait for the send/recieve threads to terminate the program
    while(1){

    }
    return 0;
}