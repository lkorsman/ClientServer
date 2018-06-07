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
	int sockfd, 
		portno, 
		n,
		bytesLeft,
	 	bytesRecv,
		bytesSent;
	long networkInt,
		 hostInt;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	char buffer[100];
	char *bp;
	bool guessedCorrect = false;
	
	
	
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
	
	
	// Get name
	printf("Please enter name: ");
	bzero(buffer, 100);
	fgets(buffer, 99, stdin);
	
	// Send name
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
		error("ERROR writing to socket");
	
	bzero(buffer,100);
	
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
		
		// Send new message
		printf("Enter a guess: ");
		cin >> hostInt;
		networkInt = htonl(hostInt);
		bytesSent = send(sockfd, (void *) &networkInt,
						 sizeof(long), 0);
		if (bytesSent != sizeof(long)) 
			exit(-1);
		
		// Receive Message Message
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
		
		if (hostInt == 0)
		{
			guessedCorrect = true;
			bzero(buffer,100);
			n = read(sockfd,buffer,99);
			if (n < 0) 
				error("ERROR reading from socket");
			printf("%s\n",buffer);
		}
		
	}
	
	
	
	
	
	close(sockfd);
	return 0;
}
