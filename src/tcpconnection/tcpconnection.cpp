#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <string.h>

#include "tcpconnection.hpp"
#include "../misc/const.hpp"
#include "../misc/misc.hpp"

ConnectionError::ConnectionError(int err){
    this->err = err;
}

std::string ConnectionError::what(){
    return strerror(this->err);
}


TCPConnection::TCPConnection(int serverFD){
    socklen_t sl;
    struct sockaddr_in caddr;
    sl = sizeof(caddr);
    this->clientFD = accept(serverFD, (struct sockaddr*)&caddr, &sl);
    if (this->clientFD == -1){
        throw ConnectionError(errno);
    }
    this->IPAddress = inet_ntoa((struct in_addr)caddr.sin_addr);
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

    return trim(message);
}

void TCPConnection::sendMessage(std::string message){
    send(this->clientFD, message.c_str(), message.length(), 0);
    // Tu rzuca broken pipe
}

TCPConnection::~TCPConnection(){
    close(this->clientFD);
}