#pragma once
#include <string>
#include <mutex>

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
    std::mutex& mutex;

    public:
        TCPConnection(int serverFD, std::mutex &m);
        TCPConnection(std::mutex &m);
        std::string getIPAddress();
        std::string getMessage();
        std::string getCurrentUser();
        void setClientFD(int fd);
        void setCurrentUser(std::string username);
        void sendMessage(std::string message);
        ~TCPConnection();
        
};