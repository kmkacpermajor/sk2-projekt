#include "commandverifier.hpp"

#include <unistd.h>

#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "../authverifier/authverifier.hpp"
#include "../misc/queries.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../tcpconnection/tcpconnection.hpp"

CommandVerifierError::CommandVerifierError(const std::string message) {
  this->message = message;
}

CommandVerifierBadArgumentsError::CommandVerifierBadArgumentsError(
    const std::string command, const std::string expected)
    : CommandVerifierError("Incorrect number of parameters for command " +
                           command + ". Expected: " + expected) {}

char const* CommandVerifierError::what() { return message.c_str(); }

CommandVerifier::CommandVerifier(SQLiteConnector& dbC) : dbConnector(dbC) {}

void CommandVerifier::verifyCommand(std::string command, paramDeque params) {
  if (commandMap.find(command) != commandMap.end()) {
    int expectedNumOfParams = commandMap[command].first;
    const std::string& expectedFormat = commandMap[command].second;

    if (params.size() != (size_t)expectedNumOfParams) {
      throw CommandVerifierBadArgumentsError(command, expectedFormat);
    }
  } else {
    throw CommandVerifierError("Unknown command: " + command);
  }

  if (command == "shutdown") {
    std::string IP = params.at(0);

    validateIpAddress(IP);
  }
}

void CommandVerifier::validateIpAddress(std::string IP) {
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