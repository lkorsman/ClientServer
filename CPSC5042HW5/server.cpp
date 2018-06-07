//
//  server.cpp
//  CPSC5042HW5
//
//  Created by Luke Korsman on 6/6/18.
//  Copyright © 2018 Luke Korsman. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <iostream>
#include <time.h>

struct Players {
	std::string name;
	int score;
};

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

long compareIntegers(int first, int second)
{
	int fDigit;
	int sDigit;
	int sum = 0;
	int difference = 0;
	
	while ( (first ) && (second ))
	{
		fDigit = first % 10;
		first  = first / 10;
		sDigit = second % 10;
		second = second / 10;
		
		if (fDigit == sDigit )
		{
			sum = sum + 0;
		}
		else
		{
			difference = abs(fDigit - sDigit);
			sum = sum + difference;
		}
	}
	return sum;
}

int main(int argc, char *argv[])
{
	int sockfd, 
		newsockfd, 
		portno,
		n;
	int turn = 1;
	socklen_t clilen;
	char buffer[100];
	struct sockaddr_in serv_addr, 
						cli_addr;
	srand(time(NULL));
	int randomNumber = rand() % 10000;
	long sum = 0;
	long networkInt;
	long hostInt;
	int bytesSent;
	int bytesRecv;
	char *bp;
	bool guessedCorrect = false;
	struct Players player1;
	Players leaderboard[3];
	
	// Initialize leaderboard to non values
	for (int i = 0; i < 3; i++)
	{
		leaderboard[i].name = "";
		leaderboard[i].score = -1;
	}
	
	
	if (argc < 2) 
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			 sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, 
					   (struct sockaddr *) &cli_addr, 
					   &clilen);
	
	if (newsockfd < 0) 
		error("ERROR on accept");
	bzero(buffer,100);
	
	printf("Random number: %d\n", randomNumber);
	
	// Read first message (user name)
	n = read(newsockfd,buffer,99);
	
	if (n < 0) 
		error("ERROR reading from socket");
	
	printf("Here is the message: %s\n",buffer);
	player1.name = buffer;
	
	bzero(buffer, 100);

	while (!guessedCorrect)
	{
		// Send turn number 
		hostInt = turn;
		networkInt = htonl(hostInt);
		bytesSent = send(newsockfd, (void *) &networkInt,
						 sizeof(long), 0);
		if (bytesSent != sizeof(long)) 
			exit(-1);
		
		// Receive Guess msg
		int bytesLeft = sizeof(long);
		bp = (char *) &networkInt;
		while (bytesLeft) {
			bytesRecv = recv(newsockfd, bp, bytesLeft, 0); 
			if (bytesRecv <= 0) 
				exit(-1);
			bytesLeft = bytesLeft - bytesRecv;
			bp = bp + bytesRecv;
		}
		hostInt = ntohl(networkInt);
		printf("Response received: %ld\n", hostInt);
		
		// Compare rand and mystery
		sum = compareIntegers(hostInt, randomNumber);
		
		// Send diff of rand and mystery
		networkInt = htonl(sum);
		bytesSent = send(newsockfd, (void *) &networkInt,
						 sizeof(long), 0);
		if (bytesSent != sizeof(long)) 
			exit(-1);
		
		if (sum == 0)
		{
			player1.score = turn;
			guessedCorrect = true;
			std::string msg = "Congratulations! It took " + std::to_string(turn) + " turns to guess the number!";
			char msgBuffer[msg.length()];
			msg.copy(msgBuffer, msg.length());
			
			leaderboard[0] = player1;
			
			std::cout << "Leader board:" << std::endl;
			for (int i = 0; i < 3; i++)
			{	
				if (leaderboard[i].score != -1)
				{
					printf("%d. %s %s", i+1, leaderboard[i].name.c_str(), std::to_string(leaderboard[i].score).c_str());
				}
				
			}
			
			
			n = write(newsockfd, msgBuffer, msg.length());
		}
		turn++;
	}
	
	
	close(newsockfd);
	close(sockfd);
	
	return 0; 
}


