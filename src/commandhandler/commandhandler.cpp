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

std::string CommandHandlerError::what() { return this->message; }

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
    SQLiteQuery checkForUser = SQLiteQuery(SELECT_USER, &dbConnector);
    checkForUser.bindText(1, username);
    auto result = checkForUser.runQuery();
    if (!result.empty()) {
      throw CommandHandlerError("User with username " + username +
                                " already exists");
    }

    SQLiteQuery insertUser = SQLiteQuery(INSERT_USER, &dbConnector);
    insertUser.bindText(1, username);
    insertUser.runOperation();
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + e.what());
  }

  return "Registered new user: " + username;
}

std::string CommandHandler::clientlistCommand(paramDeque params) {
  std::string resultString = "List of available users:\n";

  SQLiteQuery query = SQLiteQuery(SELECT_USERS, &dbConnector);
  auto rows = query.runQuery();
  for (auto row : rows) {
    resultString += row.at("username") + "\n";
  }

  return resultString;
}

std::string CommandHandler::loginCommand(paramDeque params) {
  std::string username = params[0];

  int userId;

  try {
    SQLiteQuery selectUserId = SQLiteQuery(SELECT_USER, &dbConnector);
    selectUserId.bindText(1, username);
    auto result = selectUserId.runQuery();
    if (result.empty()) {
      throw CommandHandlerError("User with username " + username +
                                " doesn't exist");
    }

    userId = std::stoi(result.at(0).at("rowid"));

    SQLiteQuery("BEGIN", &dbConnector).runOperation();
    SQLiteQuery insertMachine = SQLiteQuery(INSERT_MACHINE, &dbConnector);
    insertMachine.bindText(1, connection.getIPAddress());
    insertMachine.bindInt(2, connection.getClientFD());
    insertMachine.bindInt(3, userId);
    insertMachine.runOperation();

    SQLiteQuery insertAllowedShutdown =
        SQLiteQuery(INSERT_ALLOWED_SHUTDOWN, &dbConnector);
    insertAllowedShutdown.bindText(1, username);
    insertAllowedShutdown.bindText(2, connection.getIPAddress());
    insertAllowedShutdown.runOperation();
    SQLiteQuery("COMMIT", &dbConnector).runOperation();

  } catch (SQLiteQueryError& e) {
    SQLiteQuery("ROLLBACK", &dbConnector).runOperation();
    throw CommandHandlerError("Error: " + e.what());
  }

  connection.setCurrentUser(username, userId);

  return "Logged in as " + username;
}

std::string CommandHandler::logoffCommand(paramDeque params) {
  try {
    SQLiteQuery("BEGIN", &dbConnector).runOperation();
    // ON DELETE CASCADE didn't work as expected
    SQLiteQuery deleteAllowedShutdowns =
        SQLiteQuery(DELETE_MACHINES_ALLOWED_SHUTDOWNS, &dbConnector);
    deleteAllowedShutdowns.bindText(1, connection.getIPAddress());
    deleteAllowedShutdowns.runOperation();

    SQLiteQuery deleteMachine = SQLiteQuery(DELETE_MACHINE, &dbConnector);
    deleteMachine.bindText(1, connection.getIPAddress());
    deleteMachine.runOperation();
    SQLiteQuery("COMMIT", &dbConnector).runOperation();
  } catch (SQLiteQueryError& e) {
    SQLiteQuery("ROLLBACK", &dbConnector).runOperation();
    throw CommandHandlerError("Error: " + e.what());
  }

  connection.logout();

  return "Successfully logged off";
}

std::string CommandHandler::listCommand(paramDeque params) {
  std::string resultString = "List of available machines:\n";

  SQLiteQuery query = SQLiteQuery(SELECT_ALLOWED_SHUTDOWNS, &dbConnector);
  auto rows = query.runQuery();
  for (auto row : rows) {
    std::ostringstream formatted;
    std::string IP = row.at("ip_address");
    std::string state;
    std::string permission;

    if (row.at("state") == "1") {
      state = "ON";
    } else {
      state = "OFF";
    }

    if (row.at("permission") == "1") {
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
    SQLiteQuery insertAllowedShutdown =
        SQLiteQuery(INSERT_ALLOWED_SHUTDOWN, &dbConnector);
    insertAllowedShutdown.bindText(1, username);
    insertAllowedShutdown.bindText(2, connection.getIPAddress());
    insertAllowedShutdown.runOperation();
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + e.what());
  }

  return "Granted " + username + " permission for current machine";
}

std::string CommandHandler::revokeCommand(paramDeque params) {
  std::string username = params[0];

  try {
    SQLiteQuery deleteAllowedShutdown =
        SQLiteQuery(DELETE_ALLOWED_SHUTDOWN, &dbConnector);
    deleteAllowedShutdown.bindText(1, username);
    deleteAllowedShutdown.bindText(2, connection.getIPAddress());
    deleteAllowedShutdown.runOperation();
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + e.what());
  }

  return "Revoked permission for current machine form " + username;
}

std::string CommandHandler::shutdownCommand(paramDeque params) {
  std::string IP = params.at(0);
  int targetFD;
  try {
    SQLiteQuery selectMachine = SQLiteQuery(SELECT_MACHINE, &dbConnector);
    selectMachine.bindText(1, IP);
    auto machineResult = selectMachine.runQuery();
    if (machineResult.empty()) {
      throw AuthVerifierError("Machine with IP " + IP + " doesn't exist");
    }

    targetFD = std::stoi(machineResult.at(0).at("file_descriptor"));
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + e.what());
  }

  try {
    TCPConnection targetConnection(connection.getMutex());
    targetConnection.setClientFD(targetFD);
    targetConnection.sendMessage("close");
  } catch (ConnectionError& e) {
    throw CommandHandlerError("Error: " + e.what());
  }

  return "Machine " + IP + " successfully shutdown";
}

std::string CommandHandler::exitCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command exit. Expected: exit");
  }

  return "";  // so we don't send anything
}

std::string CommandHandler::helpCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command help. Expected: help");
  }

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