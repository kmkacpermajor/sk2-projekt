#include "tcpconnection.hpp"

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <mutex>
#include <vector>

#include "../misc/const.hpp"
#include "../misc/stringTrim.hpp"

ConnectionEndedException::ConnectionEndedException() {}

ConnectionError::ConnectionError(int err) { this->err = err; }

std::string ConnectionError::what() { return strerror(this->err); }

TCPConnection::TCPConnection(int serverFD, std::mutex& m) : mutex(m) {
  setServerFD(serverFD);
}

TCPConnection::TCPConnection(std::mutex& m) : mutex(m) {}

void TCPConnection::setServerFD(int serverFD) {
  socklen_t sl;
  struct sockaddr_in caddr;
  sl = sizeof(caddr);
  clientFD = accept(serverFD, (struct sockaddr*)&caddr, &sl);
  if (this->clientFD == -1) {
    throw ConnectionError(errno);
  }
  IPAddress = inet_ntoa((struct in_addr)caddr.sin_addr);
}

void TCPConnection::setClientFD(int fd) { clientFD = fd; }

void TCPConnection::logout() {
  currentUser = "";
  currentUserID = -1;
}

std::string TCPConnection::getIPAddress() { return IPAddress; }

std::string TCPConnection::getCurrentUser() { return currentUser; }

int TCPConnection::getCurrentUserID() { return currentUserID; }

int TCPConnection::getClientFD() { return clientFD; }

void TCPConnection::setCurrentUser(std::string username, int id) {
  currentUser = username;
  currentUserID = id;
}

std::string TCPConnection::getMessage() {
  std::vector<char> buffer(MAX_BUFFER_LENGTH);
  std::string message;
  int bytesReceived = 0;
  do {
    bytesReceived = recv(this->clientFD, &buffer[0], buffer.size(), 0);
    if (bytesReceived == -1) {
      throw ConnectionError(errno);
    } else {
      message.append(buffer.cbegin(), buffer.cend());
    }
  } while (bytesReceived == MAX_BUFFER_LENGTH);

  message.erase(std::find(message.begin(), message.end(), '\0'), message.end());

  return trim(message);
}

void TCPConnection::sendMessage(std::string message) {
  std::lock_guard<std::mutex> lock(mutex);
  int status = send(clientFD, message.c_str(), message.length(), MSG_NOSIGNAL);

  if (status == EPIPE){
    throw ConnectionEndedException();
  }
}

TCPConnection::~TCPConnection() { close(clientFD); }