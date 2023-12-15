#pragma once
#include <mutex>
#include <string>

class ConnectionEndedException : public std::exception {
 public:
  ConnectionEndedException();
};

class ConnectionError : public std::exception {
  int err;

 public:
  ConnectionError(const int err);
  char const* what();
};

class TCPConnection {
  int clientFD;
  std::string IPAddress;
  std::string currentUser = "";
  int currentUserID = -1;
  int machineID = -1;
  std::mutex& mutex;

 public:
  TCPConnection(int serverFD, std::mutex& m);
  TCPConnection(std::mutex& m);
  std::string getIPAddress();
  std::string getMessage();
  std::string getCurrentUser();
  std::mutex& getMutex();
  int getMachineID();
  int getCurrentUserID();
  int getClientFD();
  void setServerFD(int fd);
  void setClientFD(int fd);
  void setMachineID(int machineID);
  void setCurrentUser(std::string username, int id);
  void sendMessage(std::string message);
  void logout();
  ~TCPConnection();
};