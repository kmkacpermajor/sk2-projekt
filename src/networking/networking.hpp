#pragma once
#include <arpa/inet.h>

#include <string>

class ListenError : public std::exception {
  std::string message;

 public:
  ListenError(int port);
  char const* what();
};

struct sockaddr_in prepareSockAddrIn(int family, int ip, int port);