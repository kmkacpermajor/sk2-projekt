#pragma once
#include <string>

class ConnectionError : public std::exception {
    int err;
    public:
    ConnectionError(const int err);
    std::string what();
};

class TCPConnection{
    int clientFD;
    std::string IPAddress;
    std::string currentUser;

    public:
        TCPConnection(int serverFD);
        std::string getIPAddress();
        std::string getMessage();
        std::string getCurrentUser();
        void setCurrentUser(std::string username);
        void sendMessage(std::string message);
        ~TCPConnection();
        
};