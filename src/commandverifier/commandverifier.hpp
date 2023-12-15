#pragma once
#include <functional>
#include <string>
#include <deque>

#include "../misc/types.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "../authverifier/authverifier.hpp"

class CommandVerifierError : public std::exception
{
    std::string message;

public:
    CommandVerifierError(const std::string message);
    std::string what();
};

class CommandVerifierBadArgumentsError : public CommandVerifierError
{
public:
    CommandVerifierBadArgumentsError(const std::string command, const std::string expected);
};

class CommandVerifier
{
    SQLiteConnector &dbConnector;
    commandVoidFunctionMap commandFunctions;

    void helpCommand(paramDeque params);
    void registerCommand(paramDeque params);
    void loginCommand(paramDeque params);
    void logoffCommand(paramDeque params);
    void listCommand(paramDeque params);
    void grantCommand(paramDeque params);
    void revokeCommand(paramDeque params);
    void shutdownCommand(paramDeque params);
    void exitCommand(paramDeque params);
    void clientlistCommand(paramDeque params);

    void validateIpAddress(std::string IP);
    void validateUserName(std::string username);

    bool correctArgumentCountCheck(int argc, std::string command, std::string expected);
    void addCommandFunction(std::string command, const commandVoidFunction &func);

public:
    CommandVerifier(SQLiteConnector &dbC);
    void verifyCommand(std::string command, paramDeque params);
};