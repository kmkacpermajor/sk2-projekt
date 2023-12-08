#pragma once
#include <string>
#include <deque>

#include "../tcpconnection/tcpconnection.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"

class AuthVerifier
{
    TCPConnection &connection;
    SQLiteConnector &dbConnector;

    std::string commandsLoggedIn[5] = {"logoff", "list", "grant", "revoke", "shutdown"};
    std::string commandsNotLoggedIn[2] = {"register", "login"};

public:
    AuthVerifier(TCPConnection &conn, SQLiteConnector &dbC);
    bool verifyCommand(std::string command, std::deque<std::string> params);
};