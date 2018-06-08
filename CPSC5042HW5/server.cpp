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

// Struct for holding leaderboard players
struct Players {
	std::string name;
	int score;
};

// Client socket
struct ThreadArgs {
	int clientSock;
};

Players leaderboard[3]; 	// Array to hold leader board
pthread_mutex_t boardLock;	// Lock for leaderboard

// Sets leaderboard scores to 0's
void setupLeaderboard();

// System call error function that displays error message
void error(const char *msg);

// Compares client guess to random number
long compareIntegers(int first, int second);

// Interacts with client getting name and guesses
void* threadMain(void *args);

// Checks if leaderboard needs to be updated with new scores
void checkLeaderboard(int index, Players player);

// Entry point into the program
int main(int argc, char *argv[])
{
	int listenSock, // Socket server listens on
	newsockfd, 		// Socket server transmits on
	portno;			// Port of the connection
	socklen_t clilen;	// Length of the client address
	struct sockaddr_in 	serv_addr, // Address of the server
						cli_addr;	// Address of the client
	struct ThreadArgs threadArgs;	// Struct for multithreading
	pthread_t threadID;				// ID of the pthread
	
	pthread_mutex_init(&boardLock, NULL);
	
	if (argc < 2) 
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (listenSock < 0) 
		error("ERROR opening socket");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(listenSock, (struct sockaddr *) &serv_addr,
			 sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	
	// Keep listening for new clients
	while (true)
	{
		listen(listenSock,5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(listenSock, 
						   (struct sockaddr *) &cli_addr, 
						   &clilen);
		
		if (newsockfd < 0) 
			error("ERROR on accept");
		
		threadArgs.clientSock = newsockfd;
		
		int status = pthread_create(&threadID, NULL, threadMain,
									(void *) &threadArgs);
		
		if (status != 0) 
			exit(-1); // Note: status == 0 is GOOD
	}
	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

long compareIntegers(int first, int second)
{
	int fDigit;			// Holds the one's digit of first
	int sDigit;			// Holds the ones's digit of second
	int sum = 0;		// Holds the total difference of first and second
	int difference = 0;	// Holds the current difference of fDigit and sDigit
	
	while ( (first ) || (second ))
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

void* threadMain(void *args)
{
	int n;					// Return value of read/write
	int tempRandomNumber;	// Holds a temp random number
	int randomNumber;		// The final random number
	long sum;				// The difference between user guess and random num
	long networkInt;		// Integer in network byte order
	long hostInt;			// Integer in host byte order
	int bytesSent;			// bytes sent to server
	int bytesRecv;			// bytes received from server
	char *bp;				// Char pointer for integer messages
	char buffer[100];		// Store messages to be sent and received
	int turn = 1;			// Starts client guess turn at 1
	bool guessedCorrect = false;	// Bool to signal guess correct
	struct Players player1;			// Holds player info
	std::stringstream ss;			// Helps generate random num
	
	// Create 4 digit random number
	srand(time(NULL));
	for (int i = 0; i < 4; i++)
	{
		tempRandomNumber = rand() % 10;
		ss << tempRandomNumber;
	}
	ss >> randomNumber;
	
	// Extract socket file descriptor from argument
	struct ThreadArgs *threadArgs = (struct ThreadArgs *) args; 
	int clientSock = threadArgs->clientSock;	
	
	printf("Random number: %d\n", randomNumber);
	
	// Read first message (user name)
	n = read(clientSock,buffer,99);
	
	if (n < 0) 
		error("ERROR reading from socket");
	
	// Print user message
	printf("Here is the message: %s\n",buffer);
	
	player1.name = buffer;
	
	bzero(buffer, 100);
	
	// Keep interacting with client until guess is correct
	while (!guessedCorrect)
	{
		// Send turn number 
		hostInt = turn;
		networkInt = htonl(hostInt);
		bytesSent = send(clientSock, (void *) &networkInt,
						 sizeof(long), 0);
		if (bytesSent != sizeof(long)) 
			exit(-1);
		
		// Receive guess 
		int bytesLeft = sizeof(long);
		bp = (char *) &networkInt;
		while (bytesLeft) {
			bytesRecv = recv(clientSock, bp, bytesLeft, 0); 
			if (bytesRecv <= 0) 
				exit(-1);
			bytesLeft = bytesLeft - bytesRecv;
			bp = bp + bytesRecv;
		}
		hostInt = ntohl(networkInt);
		printf("Response received: %ld\n", hostInt);
		
		// Compare guess and random number
		sum = compareIntegers(hostInt, randomNumber);

		// Send diff of guess and random number
		networkInt = htonl(sum);
		bytesSent = send(clientSock, (void *) &networkInt,
						 sizeof(long), 0);
		if (bytesSent != sizeof(long)) 
			exit(-1);
		
		// If client guessed correct
		if (sum == 0)
		{
			player1.score = turn;
			guessedCorrect = true;
			
			std::string msg = "Congratulations! It took " + std::to_string(turn) + " turns to guess the number!";
			char msgBuffer[256];
			msg.copy(msgBuffer, msg.length());
			
			// Write client congrats message
			n = write(clientSock, msgBuffer, msg.length());
			bzero(msgBuffer, 256);
			
			std::cout << "Original Leaderboard\n";
			for (int j = 0; j < 3; j++)
			{
				std::cout << leaderboard[j].name << " " << leaderboard[j].score << std::endl;
			}
			
			// Lock leaderboard while updating
			pthread_mutex_lock(&boardLock);
			checkLeaderboard(0, player1);
			pthread_mutex_unlock(&boardLock);
			
			std::string leader = "\nLeader board:\n";
			for (int i = 0; i < 3; i++)
			{
				if (leaderboard[i].score != 0)
				{
					leader = leader + std::to_string(i+1) + ". " + leaderboard[i].name + " " + std::to_string(leaderboard[i].score) + "\n";
				}
			}
			leader.copy(msgBuffer, leader.length());
			n = write(clientSock, msgBuffer, leader.length());
			
			if (n < 0) 
				error("ERROR writing from socket");
			bzero(msgBuffer, 256);
		}
		turn++;
	}
	pthread_detach(pthread_self());
	close(clientSock);
	return NULL; 
}

void checkLeaderboard(int index, Players player)
{
	if (index < 3)
	{
		if (leaderboard[index].score == 0)
		{
			leaderboard[index] = player;
			return;
		}
		
		if (leaderboard[index].score > player.score)
		{
			Players temp = leaderboard[index];
			leaderboard[index] = player;
			checkLeaderboard(index + 1, temp);
		}
		else 
		{
			checkLeaderboard(index+1, player);
		}
	}
}

