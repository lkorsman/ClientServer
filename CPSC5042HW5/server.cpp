//
//  server.cpp
//  CPSC5042HW5
//
//  Created by Luke Korsman on 6/6/18.
//  Copyright © 2018 Luke Korsman. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// Display message if a system call fails
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

// Entry point into the program
int main(int argc, char *argv[])
{
	int sockfd, // Value returned from socket system call
		newsockfd, // Value returned from the accept system call
		portno,		// Port number which server accepts connections
		n;			// Return value for the read and write calls 
	socklen_t clilen;	// Size of the address of the client
	char buffer[256];	// Buffer to read characters from the socket
	struct sockaddr_in 	serv_addr, 	// Contains the internet address
						cli_addr; 	// Contains address of the client

	// Check if port number was provided on command line
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create a new socket
	
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));  // Set serv_addr buffer
													// to all 0's
	
	portno = atoi(argv[1]);	// Cast command line argument to portno integer
	serv_addr.sin_family = AF_INET;	// Set server address to Internet domain
	serv_addr.sin_addr.s_addr = INADDR_ANY; // IP address of host
	serv_addr.sin_port = htons(portno); // Convert to network byte order
	
	// Bind socket to an address, if failed display error
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			 sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	
	listen(sockfd,5); // Listen on the socket for connections
	clilen = sizeof(cli_addr);
	
	// Connect to client and communicate on newsockfd
	newsockfd = accept(sockfd, 
					   (struct sockaddr *) &cli_addr, 
					   &clilen);
	if (newsockfd < 0) 
		error("ERROR on accept");
	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s\n",buffer);
	n = write(newsockfd,"I got your message",18);
	if (n < 0) error("ERROR writing to socket");
	close(newsockfd);
	close(sockfd);
	return 0; 
}
