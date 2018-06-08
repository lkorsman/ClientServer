//
//  client.cpp
//  CPSC5042HW5
//
//  Created by Luke Korsman on 6/6/18.
//  Copyright © 2018 Luke Korsman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <iostream>

using namespace std;

// Display message if a system call fails
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, // File descriptor, stores value of socket system call
		portno, // Port number for connection
		n,		// Return value of read/write 
		bytesLeft,	// bytes left to send to server
	 	bytesRecv,	// bytes received from server
		bytesSent,	// bytes sent to server
		minGuess = 0,	// minimum value of a guess
		maxGuess = 9999;	// maximum value of a guess
	long networkInt,		// Integer in network byte order
		 hostInt;			// Integer in host byte order
	struct sockaddr_in serv_addr;  // Server address
	struct hostent *server;		// Host computer struct
	char buffer[256];			// Store messages to be sent and received
	char *bp;					// Char pointer for integer messages
	bool guessedCorrect = false;	// Bool to see if guess is correct
	
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) 
		error("ERROR opening socket");
	
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
	
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	
	
	// Get name of client user
	printf("Please enter name: ");
	bzero(buffer, 256);
	std::cin >> buffer;
	// fgets(buffer, 99, stdin);
	
	// Send name to server
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
		error("ERROR writing to socket");
	
	bzero(buffer,256);
	
	// Keep asking for guesses until correct
	while (!guessedCorrect)
	{
		// Receive Message
		bytesLeft = sizeof(long); 
		bp = (char *) &networkInt;
		while (bytesLeft) {
			bytesRecv = recv(sockfd, bp, bytesLeft, 0); 
			if (bytesRecv <= 0) 
				exit(-1);
			bytesLeft = bytesLeft - bytesRecv;
			bp = bp + bytesRecv;
		}
		hostInt = ntohl(networkInt);
		
		printf("\nTurn: %ld\n", hostInt);
		
		// Send new guess
		printf("Enter a guess: ");
		cin >> hostInt;
		
		// Check if valid guess
		while (hostInt < minGuess || hostInt > maxGuess)
		{
			printf("Invalid guess, enter a guess: ");
			cin >> hostInt;
		}
		
		// Send guess once valid
		networkInt = htonl(hostInt);
		bytesSent = send(sockfd, (void *) &networkInt,
						 sizeof(long), 0);
		if (bytesSent != sizeof(long)) 
			exit(-1);
		
		// Receive guess response message
		bytesLeft = sizeof(long); 
		bp = (char *) &networkInt;
		while (bytesLeft) {
			bytesRecv = recv(sockfd, bp, bytesLeft, 0); 
			if (bytesRecv <= 0) 
				exit(-1);
			bytesLeft = bytesLeft - bytesRecv;
			bp = bp + bytesRecv;
		}
		hostInt = ntohl(networkInt);
		
		printf("Result of guess: %ld\n", hostInt);
		
		// If guessed correct
		if (hostInt == 0)
		{
			guessedCorrect = true;
			
			// Receive congrats message
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if (n < 0) 
				error("ERROR reading from socket");
			printf("%s\n",buffer);
			
			// Receive leader message
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if (n < 0) 
				error("ERROR reading from socket");
			
			printf("%s\n", buffer);			
		}
	}
	close(sockfd);
	return 0;
}
