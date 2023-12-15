#include "networking.hpp"

#include <arpa/inet.h>

#include <string>

ListenError::ListenError(int port) {
  this->message = "Cannot listen on port: " + std::to_string(port);
}

char const* ListenError::what() { return message.c_str(); }

struct sockaddr_in prepareSockAddrIn(int family, int ip, int port) {
  struct sockaddr_in serverAddress;

  serverAddress.sin_family = family;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = htonl(ip);

  return serverAddress;
}