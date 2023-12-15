#include "commandhandler.hpp"

#include <unistd.h>

#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "../misc/queries.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../tcpconnection/tcpconnection.hpp"

CommandHandlerError::CommandHandlerError(const std::string message) {
  this->message = message;
}

char const* CommandHandlerError::what() { return message.c_str(); }

CommandHandler::CommandHandler(TCPConnection& conn, SQLiteConnector& dbC)
    : dbConnector(dbC), connection(conn) {
  addCommandFunction("help", std::bind(&CommandHandler::helpCommand, this,
                                       std::placeholders::_1));
  addCommandFunction("register", std::bind(&CommandHandler::registerCommand,
                                           this, std::placeholders::_1));
  addCommandFunction("login", std::bind(&CommandHandler::loginCommand, this,
                                        std::placeholders::_1));
  addCommandFunction("logoff", std::bind(&CommandHandler::logoffCommand, this,
                                         std::placeholders::_1));
  addCommandFunction("list", std::bind(&CommandHandler::listCommand, this,
                                       std::placeholders::_1));
  addCommandFunction("grant", std::bind(&CommandHandler::grantCommand, this,
                                        std::placeholders::_1));
  addCommandFunction("revoke", std::bind(&CommandHandler::revokeCommand, this,
                                         std::placeholders::_1));
  addCommandFunction("shutdown", std::bind(&CommandHandler::shutdownCommand,
                                           this, std::placeholders::_1));
  addCommandFunction("exit", std::bind(&CommandHandler::exitCommand, this,
                                       std::placeholders::_1));
  addCommandFunction("clientlist", std::bind(&CommandHandler::clientlistCommand,
                                             this, std::placeholders::_1));
}

void CommandHandler::addCommandFunction(std::string command,
                                        const commandStringFunction& func) {
  commandFunctions[command] = func;
}

std::string CommandHandler::handleCommand(std::string command,
                                          paramDeque params) {
  auto it = commandFunctions.find(command);
  if (it != commandFunctions.end()) {
    return it->second(params);
  } else {
    throw CommandHandlerError("Unknown command: " + command);
  }
}

std::string CommandHandler::registerCommand(paramDeque params) {
  std::string username = params[0];

  try {
    auto user =
        SQLiteQuery(SELECT_USER, &dbConnector).bindText(1, username).runQuery();
    if (!user.empty()) {
      throw CommandHandlerError("User with username " + username +
                                " already exists");
    }

    SQLiteQuery(INSERT_USER, &dbConnector).bindText(1, username).runOperation();
  } catch (SQLiteQueryError& e) {
    if (e.what_errno() == SQLITE_CONSTRAINT_UNIQUE) {
      return "User " + username + " already exists";
    }
    throw CommandHandlerError("Error: " + std::string(e.what()));
  }

  return "Registered new user: " + username;
}

std::string CommandHandler::clientlistCommand(paramDeque params) {
  std::string resultString = "List of available users:\n";

  auto users = SQLiteQuery(SELECT_USERS, &dbConnector).runQuery();
  for (auto user : users) {
    resultString += user.at("username") + "\n";
  }

  return resultString;
}

std::string CommandHandler::loginCommand(paramDeque params) {
  std::string username = params[0];

  int userID;
  int machineID;

  try {
    auto user =
        SQLiteQuery(SELECT_USER, &dbConnector).bindText(1, username).runQuery();
    if (user.empty()) {
      throw CommandHandlerError("User with username " + username +
                                " doesn't exist");
    }

    userID = std::stoi(user.at(0).at("user_id"));

    SQLiteQuery("BEGIN", &dbConnector).runOperation();
    machineID = SQLiteQuery(INSERT_MACHINE, &dbConnector)
                    .bindText(1, connection.getIPAddress())
                    .bindInt(2, connection.getClientFD())
                    .bindInt(3, userID)
                    .runOperation();

    SQLiteQuery(INSERT_ALLOWED_SHUTDOWN, &dbConnector)
        .bindInt(1, connection.getCurrentUserID())
        .bindInt(2, machineID)
        .runOperation();
    SQLiteQuery("COMMIT", &dbConnector).runOperation();

  } catch (SQLiteQueryError& e) {
    SQLiteQuery("ROLLBACK", &dbConnector).runOperation();
    throw CommandHandlerError("Error: " + std::string(e.what()));
  }

  connection.setCurrentUser(username, userID);
  connection.setMachineID(machineID);

  return "Logged in as " + username;
}

std::string CommandHandler::logoffCommand(paramDeque params) {
  try {
    SQLiteQuery("BEGIN", &dbConnector).runOperation();
    // ON DELETE CASCADE didn't work as expected
    SQLiteQuery(DELETE_MACHINES_ALLOWED_SHUTDOWNS, &dbConnector)
        .bindText(1, connection.getIPAddress())
        .runOperation();

    SQLiteQuery(DELETE_MACHINE, &dbConnector)
        .bindText(1, connection.getIPAddress())
        .runOperation();
    SQLiteQuery("COMMIT", &dbConnector).runOperation();
  } catch (SQLiteQueryError& e) {
    SQLiteQuery("ROLLBACK", &dbConnector).runOperation();
    throw CommandHandlerError("Error: " + std::string(e.what()));
  }

  connection.logout();

  return "Successfully logged off";
}

std::string CommandHandler::listCommand(paramDeque params) {
  std::string resultString = "List of available machines:\n";

  auto allowedShutdowns =
      SQLiteQuery(SELECT_ALLOWED_SHUTDOWNS, &dbConnector).runQuery();
  for (auto allowedShutdown : allowedShutdowns) {
    std::ostringstream formatted;
    std::string IP = allowedShutdown.at("ip_address");
    std::string state;
    std::string permission;

    if (allowedShutdown.at("state") == "1") {
      state = "ON";
    } else {
      state = "OFF";
    }

    if (allowedShutdown.at("permission") == "1") {
      permission = "ALLOWED";
    } else {
      permission = "NOT ALLOWED";
    }

    formatted << std::setw(15) << IP << " " << std::setw(4) << state << " "
              << std::setw(11) << std::right << permission << std::endl;

    resultString += formatted.str();
  }

  return resultString;
}

std::string CommandHandler::grantCommand(paramDeque params) {
  std::string username = params[0];

  try {
    auto userAndMachineID =
        SQLiteQuery(SELECT_USER_AND_MACHINE_ID, &dbConnector)
            .bindText(1, username)
            .bindText(2, connection.getIPAddress())
            .runQuery();

    if (userAndMachineID.empty()) {
      throw CommandHandlerError("User " + username + " doesn't exist");
    }

    SQLiteQuery(INSERT_ALLOWED_SHUTDOWN, &dbConnector)
        .bindInt(1, std::stoi(userAndMachineID.at(0).at("user_id")))
        .bindInt(2, std::stoi(userAndMachineID.at(0).at("machine_id")))
        .runOperation();
  } catch (SQLiteQueryError& e) {
    if (e.what_errno() == SQLITE_CONSTRAINT_UNIQUE) {
      return "User already has access to current machine";
    }
    throw CommandHandlerError("Error: " + std::string(e.what()));
  }

  return "Granted " + username + " permission for current machine";
}

std::string CommandHandler::revokeCommand(paramDeque params) {
  std::string username = params[0];

  try {
    SQLiteQuery(DELETE_ALLOWED_SHUTDOWN, &dbConnector)
        .bindText(1, username)
        .bindText(2, connection.getIPAddress())
        .runOperation();
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + std::string(e.what()));
  }

  return "Revoked permission for current machine form " + username;
}

std::string CommandHandler::shutdownCommand(paramDeque params) {
  std::string IP = params.at(0);
  int targetFD;
  try {
    auto machine =
        SQLiteQuery(SELECT_MACHINE, &dbConnector).bindText(1, IP).runQuery();
    if (machine.empty()) {
      throw CommandHandlerError("Machine with IP " + IP + " doesn't exist");
    }

    targetFD = std::stoi(machine.at(0).at("file_descriptor"));
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + std::string(e.what()));
  }

  try {
    TCPConnection targetConnection(connection.getMutex());
    targetConnection.setClientFD(targetFD);
    targetConnection.sendMessage("close");
  } catch (ConnectionError& e) {
    throw CommandHandlerError("Error: " + std::string(e.what()));
  }

  return "Machine " + IP + " successfully shutdown";
}

std::string CommandHandler::exitCommand(paramDeque params) {
  return "";  // so we don't send anything
}

std::string CommandHandler::helpCommand(paramDeque params) {
  std::string helpMsg =
      "- register [name] - add a new client\n"
      "- clientlist - list of available clients\n"
      "- login [name] - add current machine as [name] client's machine\n"
      "- logoff - remove current machine from [name] client's machines\n"
      "- list - list of available machines and permissions to them for current "
      "client\n"
      "- grant [name] [IP] - grant client [name] permission to shutdown "
      "machine [IP]\n"
      "- revoke [name] [IP] - revoke client's [name] permission to shutdown "
      "machine [IP]\n"
      "- shutdown [IP] - shutdown machine [IP]\n"
      "- exit - exit from application";

  return helpMsg;
}