#include "authverifier.hpp"

#include <deque>
#include <string>

#include "../misc/stringTrim.hpp"
#include "../misc/types.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../tcpconnection/tcpconnection.hpp"

AuthVerifierError::AuthVerifierError(const std::string message) {
  this->message = message;
}

std::string AuthVerifierError::what() { return this->message; }

AuthVerifier::AuthVerifier(TCPConnection &conn, SQLiteConnector &dbC)
    : connection(conn), dbConnector(dbC) {}

bool AuthVerifier::checkIfLoggedIn() {
  if (connection.getCurrentUser() == "") {
    return false;
  }
  return true;
}

void AuthVerifier::verifyCommand(std::string command, paramDeque params) {
  if (isStringInVector(commandsLoggedIn, command) && !checkIfLoggedIn()) {
    throw AuthVerifierError("This command can be used only when logged in");
  }

  if (isStringInVector(commandsNotLoggedIn, command) && checkIfLoggedIn()) {
    throw AuthVerifierError("This command can be used only when logged off");
  }

  if (command == "shutdown") {
    verifyShutdown(params.at(0));
  }
}

void AuthVerifier::verifyShutdown(std::string IP) {}