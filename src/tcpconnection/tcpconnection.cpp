#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <algorithm>
#include <mutex>

#include "tcpconnection.hpp"
#include "../misc/const.hpp"
#include "../misc/stringTrim.hpp"

ConnectionError::ConnectionError(int err){
    this->err = err;
}

std::string ConnectionError::what(){
    return strerror(this->err);
}


TCPConnection::TCPConnection(int serverFD, std::mutex &m) : mutex(m){
    socklen_t sl;
    struct sockaddr_in caddr;
    sl = sizeof(caddr);
    clientFD = accept(serverFD, (struct sockaddr*)&caddr, &sl);
    if (this->clientFD == -1){
        throw ConnectionError(errno);
    }
    IPAddress = inet_ntoa((struct in_addr)caddr.sin_addr);
}

TCPConnection::TCPConnection(std::mutex &m) : mutex(m){

}

void TCPConnection::setClientFD(int fd){
    clientFD = fd;
}

std::string TCPConnection::getIPAddress(){
    return this->IPAddress;
}

std::string TCPConnection::getCurrentUser(){
    return this->currentUser;
}

void TCPConnection::setCurrentUser(std::string username){
    this->currentUser = username;
}

std::string TCPConnection::getMessage(){
    std::vector<char> buffer(MAX_BUFFER_LENGTH);
    std::string message;   
    int bytesReceived = 0;
    do {
        bytesReceived = recv(this->clientFD, &buffer[0], buffer.size(), 0);
        if ( bytesReceived == -1 ) { 
            throw ConnectionError(errno);
        } else {
            message.append( buffer.cbegin(), buffer.cend() );
        }
    } while ( bytesReceived == MAX_BUFFER_LENGTH);

    message.erase(std::find(message.begin(), message.end(), '\0'), message.end());

    return trim(message);
}

void TCPConnection::sendMessage(std::string message){
    std::lock_guard<std::mutex> lock(mutex);
    send(this->clientFD, message.c_str(), message.length(), 0);
    // Tu rzuca broken pipe
}

TCPConnection::~TCPConnection(){
    close(this->clientFD);
}