//
//  client.cpp
//  CPSC5042HW5
//
//  Created by Luke Korsman on 6/6/18.
//  Copyright © 2018 Luke Korsman. All rights reserved.
//

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// Display message if a system call fails
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, // Value returned from socket system call
		portno, // Port number 
		n;		// Return value for the read and write calls
	struct sockaddr_in serv_addr;	// Address of the server
	struct hostent *server;		// Pointer to the hostent struct
	char buffer[256];	// Buffer for messages to be passed
	
	// Check if host name and port number were provided on command line
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	
	portno = atoi(argv[2]); // Cast command line argument to portno integer
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create a new socket
	
	// Display message if error opening socket
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	server = gethostbyname(argv[1]); // Get name of host from command line arg
	
	// Display message if server is NULL
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	// Set fields in serv_addr
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		  (char *)&serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(portno);
	
	// Display message if error connecting to host
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	
	printf("Please enter the message: ");
	bzero(buffer,256);
	fgets(buffer,255,stdin);
	n = write(sockfd,buffer,strlen(buffer));
	
	// Display message if error writing to socket
	if (n < 0) 
		error("ERROR writing to socket");
	
	bzero(buffer,256);
	n = read(sockfd,buffer,255);
	
	// Display message if error reading from socket
	if (n < 0) 
		error("ERROR reading from socket");
	
	printf("%s\n",buffer);
	close(sockfd);
	return 0;
}
