#pragma once
#include <string>

#include "../authverifier/authverifier.hpp"
#include "../commandhandler/commandhandler.hpp"
#include "../commandparser/commandparser.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"

class AgentConnection {
  TCPConnection connection;
  SQLiteConnector dbConnector;
  CommandParser commandParser;
  CommandHandler commandHandler;

 public:
  AgentConnection(int serverFD, std::mutex& m);
  void reloginUser();
  TCPConnection getConnection();
  std::string prepareResponse(std::string message);
  ~AgentConnection();
};