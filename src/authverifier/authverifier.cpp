#include <string>
#include <deque>

#include "authverifier.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"

AuthVerifierError::AuthVerifierError(const std::string message){
    this->message = message;
}

std::string AuthVerifierError::what() {
  return this->message;
}

AuthVerifier::AuthVerifier(TCPConnection &conn, SQLiteConnector &dbC) : connection(conn), dbConnector(dbC)
{
}

bool AuthVerifier::verifyCommand(std::string command, std::deque<std::string> params)
{
    return false;
}