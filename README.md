# Project: "Accio" File using TCP

This is an individual project for UNO course CSCI3550.  
This is a simple client-server application that transfers a file over a TCP connection. All implementations are written in C++ using BSD sockets.  

# Task Description
The project contains two parts: a server and a client.
- The server listens for TCP connections and saves all the received data from the client in a file.
- The client connects to the server and as soon as connection established, sends the content of a file to the server.

# Server Application Specification
The server application accepting two command-line arguments:  
$ server [PORT] [FILE-DIR]  
For example, the command below should start the server listening on port 5000 and saving received files in the directory /save  
$ ./server 5000 /save

# Client Application Specification
The client application accepting three command-line arguments:  
$ ./client [HOSTNAME-OR-IP] [PORT] [FILENAME]  
For example, the command below should result in connection to a server on the same machine listening on port 5000 and transfer content of file.txt:  
$ ./client localhost 5000 file.txt  

# Project Requirements
- At least 3 git commits
- Client handles incorrect hostname/port
- Server handles incorrect port
- Server handles SIGTERM / SIGQUIT signals
- Client connects and starts transmitting a file
- Server accepts a connection and start saving a file
- Client able to successfully transmit a small file (500 bytes)
- Client able to successfully transmit a medium size file (1 MiB) (sending in large chunks without delays)
- Client able to successfully transmit a large size file (100 MiB) (sending in large chunks without delay)
- Server able to receive a small file (500 bytes) and save it in 1.file
- Server able to receive a medium file (1 MiB bytes) and save it in 1.file (receiving file sent in large chunks without delays, receiving file sent in small chunks with delays)
- Server able to receive a large file (100 MiB bytes) and save it in 1.file (receiving file sent in large chunks without delays, receiving file sent in small chunks with delays)
- Server can properly receive 10 small files (sent without delays) in 1.file, 2.file, … 10.file when a single client connects sequentially
- Client handles abort connection attempt after 10 seconds.
- Client aborts connection when server gets disconnected (server app or network connection is down)
- Server aborts connection (a file should be created, containing only ERROR string) when doesn’t receive data from client for more than 10 seconds
- Client able to successfully send and server properly receive and save large file over lossy and large delay network (we will use tc based emulation).
