#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <signal.h>

// reads chunks of 1024 bytes
#define BUFFER_SIZE 1024

// METHOD for handling SIGQUIT/SIGTERM signal
void signalHandler(int signum)
{
  exit(0);
}

int
main(int argc, char* argv[])
{

  int count = 1;
  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  // register signal and signal handler
  signal(SIGQUIT, signalHandler);
  signal(SIGTERM, signalHandler);

  // check for valid number of arguments
  if (argc < 3) {
    std::cerr << "ERROR: Invalid number of arguments" << std::endl;
    exit(1);
  }

  // check for valid port
  int port = atoi(argv[1]); // string to int (port number)
  if (port <= 1023) {
    std::cerr << "ERROR: Invalid port" << std::endl;
    exit(1);
  }

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // server aborts connection when doesn’t receive data from client?
  setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  startlisten:
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  // accept a new connection
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);
  int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

  if (clientSockfd == -1) {
    perror("accept");
    return 4;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // where to save the received files
  std::string directory = argv[2];
  std::string filename = std::to_string(count);
  count++;
  std::string path = directory + "/" + filename + ".file";
  std::ofstream outFile(path, std::ios::binary);

  // read/write data from/into the connection
  char buffer[BUFFER_SIZE] = {0};

  while (1) {
    memset(buffer, '\0', sizeof(buffer));

    int bytes = recv(clientSockfd, buffer, sizeof(buffer), 0);

    // file received successfully
    if (bytes == 0) {
      break;
    }

    // data is not complete
    if (bytes == -1) {
      // clear file before writing
      outFile.close();
      std::ofstream outFile (path, std::ios::out | std::ios::trunc);

      std::string err = "ERROR";
      outFile.write(err.c_str(), err.size());

      perror("recv");
      return 5;
    }

    outFile.write(buffer, bytes);
  }


  outFile.close();
  close(clientSockfd);

  goto startlisten;

  return 0;
}