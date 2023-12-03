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

class CommandHandlerBadArgumentsError : public CommandHandlerError{
    public:
    CommandHandlerBadArgumentsError(const std::string command, const std::string expected);
};

class CommandHandler{
    SQLiteConnector& dbConnector;
    TCPConnection& connection;
    commandFunctionMap commandFunctions;

    std::string helpCommand(paramDeque params);
    std::string registerCommand(paramDeque params);
    std::string loginCommand(paramDeque params);
    std::string logoffCommand(paramDeque params);
    std::string listCommand(paramDeque params);
    std::string grantCommand(paramDeque params);
    std::string revokeCommand(paramDeque params);
    std::string shutdownCommand(paramDeque params);
    std::string exitCommand(paramDeque params);
    std::string clientlistCommand(paramDeque params);

    bool correctArgumentCountCheck(int argc, std::string command, std::string expected);

    public:
        CommandHandler(TCPConnection &conn, SQLiteConnector &dbC);
        void addCommandFunction(std::string command, const commandFunction& func);
        std::string handleCommand(std::string command, paramDeque params);
        ~CommandHandler();
};