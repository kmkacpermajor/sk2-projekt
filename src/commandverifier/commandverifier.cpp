#include <unistd.h>
#include <deque>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>

#include "../authverifier/authverifier.hpp"
#include "../misc/queries.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "commandverifier.hpp"

CommandVerifierError::CommandVerifierError(const std::string message) {
  this->message = message;
}

CommandVerifierBadArgumentsError::CommandVerifierBadArgumentsError(
    const std::string command,
    const std::string expected)
    : CommandVerifierError("Incorrect number of parameters for command " +
                          command + ". Expected: " + expected) {}

std::string CommandVerifierError::what() {
  return this->message;
}

CommandVerifier::CommandVerifier(SQLiteConnector& dbC)
    : dbConnector(dbC) {
  addCommandFunction("help", std::bind(&CommandVerifier::helpCommand, this,
                                       std::placeholders::_1));
  addCommandFunction("register", std::bind(&CommandVerifier::registerCommand,
                                           this, std::placeholders::_1));
  addCommandFunction("login", std::bind(&CommandVerifier::loginCommand, this,
                                        std::placeholders::_1));
  addCommandFunction("logoff", std::bind(&CommandVerifier::logoffCommand, this,
                                         std::placeholders::_1));
  addCommandFunction("list", std::bind(&CommandVerifier::listCommand, this,
                                       std::placeholders::_1));
  addCommandFunction("grant", std::bind(&CommandVerifier::grantCommand, this,
                                        std::placeholders::_1));
  addCommandFunction("revoke", std::bind(&CommandVerifier::revokeCommand, this,
                                         std::placeholders::_1));
  addCommandFunction("shutdown", std::bind(&CommandVerifier::shutdownCommand,
                                           this, std::placeholders::_1));
  addCommandFunction("exit", std::bind(&CommandVerifier::exitCommand, this,
                                       std::placeholders::_1));
  addCommandFunction("clientlist", std::bind(&CommandVerifier::clientlistCommand,
                                             this, std::placeholders::_1));
}

void CommandVerifier::addCommandFunction(std::string command,
                                        const commandVoidFunction& func) {
  commandFunctions[command] = func;
}

void CommandVerifier::verifyCommand(std::string command,
                                          paramDeque params) {
  auto it = commandFunctions.find(command);
  if (it != commandFunctions.end()) {
    it->second(params);
  } else {
    throw CommandVerifierError("Unknown command: " + command);
  }
}

void CommandVerifier::validateIpAddress(std::string IP){
  std::vector<int> octets;
  std::istringstream iss(IP);
  std::string octet;

  while (std::getline(iss, octet, '.')) {
      try {
          int value = std::stoi(octet);
          if (value < 0 || value > 255) {
              throw CommandVerifierError("Invalid IP address given");
          }
          octets.push_back(value);
      } catch (std::invalid_argument&) {
          throw CommandVerifierError("Invalid IP address given");
      } catch (std::out_of_range&) {
          throw CommandVerifierError("Invalid IP address given");
      }
  }

  if (octets.size() != 4) {
      throw CommandVerifierError("Invalid IP address given");
  }
}

void CommandVerifier::validateUserName(std::string username){
  try {
    SQLiteQuery checkForUser = SQLiteQuery(SELECT_USER, &dbConnector);
    checkForUser.bindText(1, username);
    auto result = checkForUser.runQuery();
    if (result.empty()){
      throw CommandVerifierError("User with username "+username+" doesn't exist");
    }
  } catch (SQLiteQueryError& e) {
    throw CommandVerifierError("Error: " + e.what());
  }
}

void CommandVerifier::registerCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandVerifierBadArgumentsError(
        "register", "register [name]");
  }
}

void CommandVerifier::clientlistCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandVerifierBadArgumentsError(
        "clientlist", "clientlist");
  }
}

void CommandVerifier::loginCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandVerifierBadArgumentsError(
        "login", "login [name]");
  }

  std::string username = params[0];

  validateUserName(username);
}

void CommandVerifier::logoffCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandVerifierBadArgumentsError(
        "logoff", "logoff");
  }
}

void CommandVerifier::listCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandVerifierBadArgumentsError(
        "list", "list");
  }
}

void CommandVerifier::grantCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandVerifierBadArgumentsError(
        "grant", "grant [name]");
  }

  std::string username = params[0];

  validateUserName(username);
}

void CommandVerifier::revokeCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandVerifierBadArgumentsError(
        "revoke", "revoke [name]");
  }

  std::string username = params[0];

  validateUserName(username);
}

void CommandVerifier::shutdownCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandVerifierBadArgumentsError(
        "shutdown", "shutdown [IP]");
  }

  std::string IP = params.at(0);

  validateIpAddress(IP);
}

void CommandVerifier::exitCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandVerifierBadArgumentsError(
        "exit", "exit");
  }

}

void CommandVerifier::helpCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandVerifierBadArgumentsError(
        "help", "help");
  }

}
