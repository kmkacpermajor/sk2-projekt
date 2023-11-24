#pragma once
#include <functional>
#include <string>
#include <deque>

#include "../misc/types.hpp"
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
    commandFunctionMap commandFunctions;

    std::string helpCommand(paramDeque params);
    public:
        CommandHandler(TCPConnection &conn, SQLiteConnector &dbC);
        void addCommandFunction(std::string command, const commandFunction& func);
        std::string handleCommand(std::string command, paramDeque params);
        ~CommandHandler();
};