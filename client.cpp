#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <fcntl.h>
#include <sys/select.h>

// reads chunks of 1024 bytes
#define BUFFER_SIZE 1024

int
main(int argc, char *argv[])
{
  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  // check for valid number of arguments
  if (argc < 4) {
    std::cerr << "ERROR: Invalid number of arguments" << std::endl;
    exit(1);
  }

  // check for valid host name
  struct hostent* host = gethostbyname(argv[1]);
  if (host == NULL) {
    std::cerr << "ERROR: Invalid host" << std::endl;
    exit(1);
  }

  // check for valid port number
  int port = atoi(argv[2]);
  if (port <= 1023) {
    std::cerr << "ERROR: Invalid port" << std::endl;
    exit(1);
  }

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // binds the socket to the address and port number
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = *(long*)(host->h_addr_list[0]);
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  /////////////////////////////////////////////////////////////////////
  // set socket non-blocking
  int flags = fcntl (sockfd, F_GETFL, 0);
  fcntl (sockfd, F_SETFL, flags | O_NONBLOCK);

  // connect to the server
  int n = connect (sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

  if (n == -1) {
    if (errno != EINPROGRESS) {
      perror("connect");
      exit(-1);
    }

    fd_set wset, rset;
    FD_ZERO (&rset);
    FD_SET (sockfd, &rset);
    wset = rset;

    // wait for socket to be writable; return after given timeout
    n = select (sockfd + 1, NULL, &wset, &rset, &timeout);
    if (n == 0) {
      std::cerr << "ERROR: timeout" << std::endl;
      close(sockfd);
      errno = ETIMEDOUT;
      exit(-1);
    }
  }
  ////////////////////////////////////////////////////////////////////
 
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }


  // timeout for not being able to send more data to server?
  if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    std::cerr << "ERROR: timeout" << std::endl;
    close(sockfd);
    exit(-1);
  }


  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;

  // send/receive data to/from connection
  char buffer[BUFFER_SIZE] = {0};
  std::ifstream inFile(argv[3], std::ios::binary);

  while (1) {
    memset(buffer, '\0', sizeof(buffer));

    // read file
    inFile.read(buffer, sizeof(buffer));

    // break out of loop if nothing read
    if (inFile.gcount() == 0)
    {
      break;
    }

    // send data
    if (send(sockfd, buffer, inFile.gcount(), 0) == -1) {
      perror("send");
      return 4;
    }
  }

  inFile.close();
  close(sockfd);

  return 0;
}