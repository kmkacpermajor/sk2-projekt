#include "authverifier.hpp"

#include <deque>
#include <string>

#include "../misc/queries.hpp"
#include "../misc/stringopers.hpp"
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

void AuthVerifier::authCommand(std::string command, paramDeque params) {
  if (isStringInVector(commandsLoggedIn, command) && !checkIfLoggedIn()) {
    throw AuthVerifierError("This command can be used only when logged in");
  }

  if (isStringInVector(commandsNotLoggedIn, command) && checkIfLoggedIn()) {
    throw AuthVerifierError("This command can be used only when logged off");
  }

  if (command == "shutdown") {
    authShutdown(params.at(0));
  }
}

void AuthVerifier::authShutdown(std::string IP) {
  try {
    auto machineResult =
        SQLiteQuery(SELECT_MACHINE, &dbConnector).bindText(1, IP).runQuery();
    if (machineResult.empty()) {
      throw AuthVerifierError("Machine with IP " + IP + " doesn't exist");
    }

    auto allowedShutdownResult =
        SQLiteQuery(SELECT_ALLOWED_SHUTDOWN, &dbConnector)
            .bindText(1, connection.getCurrentUser())
            .bindText(2, IP)
            .runQuery();

    if (allowedShutdownResult.empty()) {
      throw AuthVerifierError(
          "Current user doesn't have permission to shutdown machine " + IP);
    }

  } catch (SQLiteQueryError &e) {
    throw AuthVerifierError("Error: " + e.what());
  }
}