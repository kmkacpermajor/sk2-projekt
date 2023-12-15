#include "agentconnection.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

#include "../authverifier/authverifier.hpp"
#include "../commandhandler/commandhandler.hpp"
#include "../commandparser/commandparser.hpp"
#include "../misc/const.hpp"
#include "../misc/queries.hpp"
#include "../misc/stringopers.hpp"
#include "../misc/types.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../tcpconnection/tcpconnection.hpp"

AgentConnection::AgentConnection(int serverFD, std::mutex &m)
    : connection(serverFD, m),
      dbConnector(DB_NAME),
      commandHandler(connection, dbConnector) {
  reloginUser();

  if (connection.getCurrentUser() == "") {
    connection.sendMessage("Connected to server");
  } else {
    connection.sendMessage("Connected to server logged in as " +
                           connection.getCurrentUser());
  }
}

void AgentConnection::reloginUser() {
  try {
    SQLiteQuery(SET_FD, &dbConnector)
        .bindInt(1, connection.getClientFD())
        .bindText(2, connection.getIPAddress())
        .runOperation();

    try {
      auto ownerRow = SQLiteQuery(SELECT_MACHINE_OWNER, &dbConnector)
                          .bindText(1, connection.getIPAddress())
                          .runQuery()
                          .at(0);
      SQLiteQuery(SET_STATUS, &dbConnector)
          .bindInt(1, 1)
          .bindText(2, connection.getIPAddress())
          .runOperation();
      connection.setCurrentUser(ownerRow.at("username"),
                                std::stoi(ownerRow.at("id")));
    } catch (std::out_of_range &e) {
      std::cout << "No user for current machine. Not relogging" << std::endl;
    }
  } catch (SQLiteQueryError &e) {
    std::cout << "Error occurred when trying to relogin: " << e.what()
              << std::endl;
  }
}

TCPConnection &AgentConnection::getConnection() { return connection; }

std::string AgentConnection::prepareResponse(std::string message) {
  try {
    if (message.empty()) {
      return "";
    } else {
      paramDeque params = commandParser.parseCommand(message);

      std::string command = params.front();
      params.pop_front();

      AuthVerifier(connection, dbConnector).verifyCommand(command, params);

      return commandHandler.handleCommand(command, params);
    }
  } catch (CommandHandlerError &e) {
    std::cout << "Error occurred when handling command: " << message << ": "
              << e.what() << std::endl;
    return e.what();
  } catch (CommandParserError &e) {
    std::cout << "Error occurred when parsing command: " << message << ": "
              << e.what() << std::endl;
    return e.what();

  } catch (AuthVerifierError &e) {
    std::cout << "Error occurred when authorizing command: " << message << ": "
              << e.what() << std::endl;
    return e.what();
  }
}

AgentConnection::~AgentConnection() {}
