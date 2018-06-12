To run this program on CS1:

To compile server.cpp enter the following on the command line
g++ server.cpp -lpthread -o server.out

To compile client.cpp enter the following on the command line
g++ client.cpp -o client.out


To execute the server.cpp program, enter the following on the command line
./server.out 88888

To execute the client.cpp program, enter the following on the command line
./client.out 10.124.72.20 88888

________________

Notes:
- These files must be compiled with C++11
- The server program (server.cpp) must be executed first to open a listening socket
- The server program (server.cpp) must be executed in a different terminal window than the client.cpp programs
- On the execution line of the server, 88888 refers to the port number for the server connection
- On the execution line of the client, 10.124.72.20 is the IP address for CS1
- On the execution line of the client, 88888 refers to the same port number as the server
- When prompted for the player's name, do not include any spaces nor numbers
- The server program will run indefinitely and will need to be stopped by using Ctrl-C
