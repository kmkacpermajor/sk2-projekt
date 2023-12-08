#include <string>
#include <deque>

#include "authverifier.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"

AuthVerifier::AuthVerifier(TCPConnection &conn, SQLiteConnector &dbC) : connection(conn), dbConnector(dbC)
{
}

bool AuthVerifier::verifyCommand(std::string command, std::deque<std::string> params)
{
    return false;
}