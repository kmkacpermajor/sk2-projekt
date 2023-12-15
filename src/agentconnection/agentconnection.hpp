#pragma once
#include <string>

#include "../authverifier/authverifier.hpp"
#include "../commandhandler/commandhandler.hpp"
#include "../commandparser/commandparser.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "../commandverifier/commandverifier.hpp"

class AgentConnection {
  TCPConnection connection;
  SQLiteConnector dbConnector;
  CommandParser commandParser;
  CommandHandler commandHandler;
  AuthVerifier authVerifier;
  CommandVerifier commandVerifier;

 public:
  AgentConnection(int serverFD, std::mutex& m);
  void reloginUser();
  TCPConnection& getConnection();
  std::string prepareResponse(std::string message);
  ~AgentConnection();
};