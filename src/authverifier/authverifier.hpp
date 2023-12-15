#pragma once
#include <string>
#include <deque>

#include "../tcpconnection/tcpconnection.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"

class AuthVerifierError : public std::exception {
    std::string message;
    public:
    AuthVerifierError(const std::string message);
    std::string what();
};

class AuthVerifier
{
    TCPConnection &connection;
    SQLiteConnector &dbConnector;

    std::vector<std::string> commandsLoggedIn {"logoff", "list", "grant", "revoke", "shutdown"};
    std::vector<std::string> commandsNotLoggedIn {"register", "login"};

    bool checkIfLoggedIn();
    void authShutdown(std::string params);

public:
    AuthVerifier(TCPConnection &conn, SQLiteConnector &dbC);
    void authCommand(std::string command, std::deque<std::string> params);
};