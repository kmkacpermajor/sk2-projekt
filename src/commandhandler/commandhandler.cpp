#include <string>

#include "commandhandler.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"

CommandHandlerError::CommandHandlerError(const std::string message){
    this->message = message;
}

std::string CommandHandlerError::what(){
    return this->message;
}

CommandHandler::CommandHandler(TCPConnection &conn, SQLiteConnector &dbC) : dbConnector(dbC), connection(conn){
    
}

CommandHandler::~CommandHandler(){
    
}