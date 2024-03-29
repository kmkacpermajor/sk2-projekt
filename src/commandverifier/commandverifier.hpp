#pragma once
#include <deque>
#include <functional>
#include <string>

#include "../authverifier/authverifier.hpp"
#include "../misc/types.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"

class CommandVerifierError : public std::exception {
  std::string message;

 public:
  CommandVerifierError(const std::string message);
  char const *what();
};

class CommandVerifierBadArgumentsError : public CommandVerifierError {
 public:
  CommandVerifierBadArgumentsError(const std::string command,
                                   const std::string expected);
};

class CommandVerifier {
  SQLiteConnector &dbConnector;

  std::unordered_map<std::string, std::pair<int, std::string>> commandMap = {
      {"register", {1, "register [name]"}},
      {"clientlist", {0, "clientlist"}},
      {"login", {1, "login [name]"}},
      {"logoff", {0, "login"}},
      {"list", {0, "list"}},
      {"grant", {1, "grant [name]"}},
      {"revoke", {1, "revoke [name]"}},
      {"shutdown", {1, "shutdown [IP]"}},
      {"exit", {0, "exit"}},
      {"help", {0, "help"}}};

  void validateIpAddress(std::string IP);

 public:
  CommandVerifier(SQLiteConnector &dbC);
  void verifyCommand(std::string command, paramDeque params);
};