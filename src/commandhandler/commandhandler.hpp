#pragma once
#include <deque>
#include <functional>
#include <string>

#include "../authverifier/authverifier.hpp"
#include "../misc/types.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"

class CommandHandlerError : public std::exception {
  std::string message;

 public:
  CommandHandlerError(const std::string message);
  char const *what();
};

class CommandHandler {
  SQLiteConnector &dbConnector;
  TCPConnection &connection;
  commandStringFunctionMap commandFunctions;

  std::string helpCommand(paramDeque params);
  std::string registerCommand(paramDeque params);
  std::string loginCommand(paramDeque params);
  std::string logoffCommand(paramDeque params);
  std::string listCommand(paramDeque params);
  std::string grantCommand(paramDeque params);
  std::string revokeCommand(paramDeque params);
  std::string shutdownCommand(paramDeque params);
  std::string exitCommand(paramDeque params);
  std::string clientlistCommand(paramDeque params);

  bool correctArgumentCountCheck(int argc, std::string command,
                                 std::string expected);
  void addCommandFunction(std::string command,
                          const commandStringFunction &func);

 public:
  CommandHandler(TCPConnection &conn, SQLiteConnector &dbC);
  std::string handleCommand(std::string command, paramDeque params);
};