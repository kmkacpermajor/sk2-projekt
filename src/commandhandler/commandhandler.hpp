#pragma once
#include <string>
#include <memory>

#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"

class CommandHandlerError : public std::exception {
    std::string message;
    public:
    CommandHandlerError(const std::string message);
    std::string what();
};

class CommandHandler{
    SQLiteConnector& dbConnector;
    TCPConnection& connection;
    public:
        CommandHandler(TCPConnection &conn, SQLiteConnector &dbC);
        ~CommandHandler();
};