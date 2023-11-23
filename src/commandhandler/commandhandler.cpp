#include <string>
#include <sstream>
#include <iostream>

#include "commandhandler.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"

CommandHandlerError::CommandHandlerError(const std::string message){
    this->message = message;
}

std::string CommandHandlerError::what(){
    return this->message;
}

CommandHandler::CommandHandler(TCPConnection conn, SQLiteConnector dbC) : dbConnector(dbC), connection(conn){
    
}

std::string CommandHandler::handleCommand(){
  std::cout << this->connection.getCurrentUser() << std::endl;
    // std::istringstream iss(command);
    // std::string s;
    // while (getline(iss, s, ' ')){
    //     std::cout << s << std::endl;
    // }
}

CommandHandler::~CommandHandler(){
    std::cout << "cos" << std::endl;
}