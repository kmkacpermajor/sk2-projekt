#include <unistd.h>
#include <deque>
#include <map>
#include <string>

#include "../authverifier/authverifier.hpp"
#include "../misc/queries.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "commandhandler.hpp"

CommandHandlerError::CommandHandlerError(const std::string message) {
  this->message = message;
}

CommandHandlerBadArgumentsError::CommandHandlerBadArgumentsError(
    const std::string command,
    const std::string expected)
    : CommandHandlerError("Incorrect number of parameters for command " +
                          command + ". Expected: " + expected) {}

std::string CommandHandlerError::what() {
  return this->message;
}

CommandHandler::CommandHandler(TCPConnection& conn, SQLiteConnector& dbC)
    : dbConnector(dbC), connection(conn), authVerifier(conn, dbC) {
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
                                        const commandFunction& func) {
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
  if (params.size() != 1) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command register. Expected: "
        "register [name]");
  }

  std::string username = params[0];

  try {
    SQLiteQuery checkForUser = SQLiteQuery(SELECT_USER_ID, &dbConnector);
    checkForUser.bindText(1, username);
    auto result = checkForUser.runQuery();
    if (!result.empty()){
      throw CommandHandlerError("User with username "+username+" already exists");
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
  if (params.size() != 0) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command clientlist. Expected: "
        "clientlist");
  }

  std::string resultString = "List of available users:\n";

  SQLiteQuery query = SQLiteQuery(SELECT_USERS, &dbConnector);
  auto rows = query.runQuery();
  for (auto row : rows) {
    resultString += row.at("username") + "\n";
  }

  return resultString;
}

std::string CommandHandler::loginCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command login. Expected: login "
        "[name]");
  }

  std::string username = params[0];

  int userId;

  try{
    SQLiteQuery selectUserId = SQLiteQuery(SELECT_USER_ID, &dbConnector);
    selectUserId.bindText(1, username);
    auto result = selectUserId.runQuery();
    if (result.empty()){
      throw CommandHandlerError("User with username "+username+" doesn't exist");
    }

    userId = std::stoi(result.at(0).at("rowid"));

    SQLiteQuery("BEGIN", &dbConnector).runOperation();
    SQLiteQuery insertMachine = SQLiteQuery(INSERT_MACHINE, &dbConnector);
    insertMachine.bindText(1, connection.getIPAddress());
    insertMachine.bindInt(2, connection.getClientFD());
    insertMachine.bindInt(3, userId);
    insertMachine.runOperation();

    SQLiteQuery insertAllowedShutdown = SQLiteQuery(INSERT_ALLOWED_SHUTDOWN, &dbConnector);
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
  if (params.size() != 0) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command logoff. Expected: logoff");
  }

  try{
    SQLiteQuery("BEGIN", &dbConnector).runOperation();
    // ON DELETE CASCADE didn't work as expected
    SQLiteQuery deleteAllowedShutdowns = SQLiteQuery(DELETE_MACHINES_ALLOWED_SHUTDOWNS, &dbConnector);
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
  if (params.size() != 0) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command list. Expected: list");
  }

  // TODO add format string for better printing
  std::string resultString = "List of available machines:\n";

  SQLiteQuery query = SQLiteQuery(SELECT_ALLOWED_SHUTDOWNS, &dbConnector);
  auto rows = query.runQuery();
  for (auto row : rows) {
    resultString += row.at("ip_address");
    if (row.at("state") == "1"){
      resultString += " ON ";
    }else{
      resultString += " OFF";
    }

    if (row.at("permission") == "1"){
      resultString += " ALLOWED\n";
    }else{
      resultString += " NOT ALLOWED\n";
    }
  }

  return resultString;
}

std::string CommandHandler::grantCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command grant. Expected: grant "
        "[name]");
  }

  std::string username = params[0];

  try{
    SQLiteQuery selectUserId = SQLiteQuery(SELECT_USER_ID, &dbConnector);
    selectUserId.bindText(1, username);
    auto result = selectUserId.runQuery();
    if (result.empty()){
      throw CommandHandlerError("User with username "+username+" doesn't exist");
    }
    
    SQLiteQuery insertAllowedShutdown = SQLiteQuery(INSERT_ALLOWED_SHUTDOWN, &dbConnector);
    insertAllowedShutdown.bindText(1, username);
    insertAllowedShutdown.bindText(2, connection.getIPAddress());
    insertAllowedShutdown.runOperation();
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + e.what());
  }

  return "Granted "+username+" permission for current machine";
}

std::string CommandHandler::revokeCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command revoke. Expected: revoke "
        "[name]");
  }

  std::string username = params[0];

  try{
    SQLiteQuery selectUserId = SQLiteQuery(SELECT_ALLOWED_SHUTDOWN, &dbConnector);
    selectUserId.bindText(1, username);
    selectUserId.bindText(2, connection.getIPAddress());
    auto result = selectUserId.runQuery();
    if (result.empty()){
      throw CommandHandlerError("User with username "+username+" doesn't have permissions for current machine");
    }

    SQLiteQuery deleteAllowedShutdown = SQLiteQuery(DELETE_ALLOWED_SHUTDOWN, &dbConnector);
    deleteAllowedShutdown.bindText(1, username);
    deleteAllowedShutdown.bindText(2, connection.getIPAddress());
    deleteAllowedShutdown.runOperation();
  } catch (SQLiteQueryError& e) {
    throw CommandHandlerError("Error: " + e.what());
  }

  return "Revoked permission for current machine form "+username;
}

std::string CommandHandler::shutdownCommand(paramDeque params) {
  if (params.size() != 1) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command shutdown. Expected: "
        "shutdown [IP]");
  }

  return "SHUTDOWN";
}

std::string CommandHandler::exitCommand(paramDeque params) {
  if (params.size() != 0) {
    throw CommandHandlerError(
        "Incorrect number of parameters for command exit. Expected: exit");
  }

  return ""; // so we don't send anything
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

CommandHandler::~CommandHandler() {
  SQLiteQuery deleteAllowedShutdown = SQLiteQuery(FINALIZE_MACHINE, &dbConnector);
  deleteAllowedShutdown.bindText(1, connection.getIPAddress());
  deleteAllowedShutdown.runOperation();
}