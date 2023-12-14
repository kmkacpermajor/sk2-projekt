#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>
#include <deque>
#include <iostream>
#include <thread>
#include <vector>

#include "authverifier/authverifier.hpp"
#include "commandhandler/commandhandler.hpp"
#include "commandparser/commandparser.hpp"
#include "misc/const.hpp"
#include "misc/queries.hpp"
#include "misc/stringTrim.hpp"
#include "misc/types.hpp"
#include "networking/networking.hpp"
#include "sqliteconnector/sqliteconnector.hpp"
#include "sqlitequery/sqlitequery.hpp"
#include "tcpconnection/tcpconnection.hpp"

std::mutex m;

void handleNewServerConnection(int serverFD) {
  auto connection = TCPConnection(serverFD, m);
  auto dbConnector = SQLiteConnector(DB_NAME);
  auto commandParser = CommandParser();
  auto commandHandler = CommandHandler(connection, dbConnector);

  // sprawdzenie maszyny czy ma fd, jak tak to zamykamy go (co z wątkiem)
  // refactor góry i dołu

  try {
    SQLiteQuery(SET_FD, &dbConnector)
        .bindInt(1, connection.getClientFD())
        ->bindText(2, connection.getIPAddress())
        ->runOperation();

    try {
      auto ownerRow = SQLiteQuery(SELECT_MACHINE_OWNER, &dbConnector)
                          .bindText(1, connection.getIPAddress())
                          ->runQuery()
                          .at(0);
      SQLiteQuery(SET_STATUS, &dbConnector)
          .bindInt(1, 1)
          ->bindText(2, connection.getIPAddress())
          ->runOperation();
      connection.setCurrentUser(ownerRow.at("username"),
                                std::stoi(ownerRow.at("id")));
    } catch (std::out_of_range &e) {
      std::cout << "No user for current machine. Not relogging" << std::endl;
    }
  } catch (SQLiteQueryError &e) {
    std::cout << "Error occurred when trying to relogin: " << e.what()
              << std::endl;
  }

  if (connection.getCurrentUser() == "") {
    connection.sendMessage("Connected to server");
  } else {
    connection.sendMessage("Connected to server logged in as " +
                           connection.getCurrentUser());
  }

  while (1) {
    std::string message;
    std::string response;
    try {
      message = connection.getMessage();
    } catch (ConnectionError &e) {
      std::cout << "Error occurred when getting new message: " << e.what()
                << std::endl;
      break;
    }

    try {
      if (message.empty()) {
        response = "";
      } else {
        paramDeque params = commandParser.parseCommand(message);

        std::string command = params.front();
        params.pop_front();

        AuthVerifier(connection, dbConnector).verifyCommand(command, params);

        response = commandHandler.handleCommand(command, params);
      }
    } catch (CommandHandlerError &e) {
      response = e.what();
      std::cout << "Error occurred when handling command: " << message << ": "
                << e.what() << std::endl;
    } catch (CommandParserError &e) {
      response = e.what();
      std::cout << "Error occurred when parsing command: " << message << ": "
                << e.what() << std::endl;
    } catch (AuthVerifierError &e) {
      response = e.what();
      std::cout << "Error occurred when authorizing command: " << message
                << ": " << e.what() << std::endl;
    }

    if (response.length() == 0) {
      return;
    }

    try {
      connection.sendMessage(trim(response));
    } catch (ConnectionError &e) {
      std::cout << "Error occurred when sending response: " << e.what()
                << std::endl;
    } catch (ConnectionEndedException &e) {
      return;
    }
  }
}

void *handleNewThread(void *arg) {
  int *serverFD = (int *)arg;

  try {
    while (1) {
      handleNewServerConnection(*serverFD);
    }
  } catch (ConnectionError &e) {
    std::cout << "Error occurred when connecting: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (DatabaseError &e) {
    std::cout << "Failed to create database: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main() {
  try {
    SQLiteConnector initSQLiteConnector = SQLiteConnector(DB_NAME);
    initSQLiteConnector.initDatabase();
  } catch (DatabaseError &e) {
    std::cout << "Error occurred when initializing database: " << e.what()
              << std::endl;
    exit(EXIT_FAILURE);
  }

  int serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serverAddress =
      prepareSockAddrIn(AF_INET, INADDR_ANY, PORT);

  bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  if (listen(serverSocket, QUEUE_SIZE) != 0) {
    throw ListenError(PORT);
  }

  std::cout << "Listening on port " << PORT << std::endl;

  for (int i = 0; i < EXTRA_THREADS_COUNT; i++) {
    std::thread thread(handleNewThread, &serverSocket);
    thread.detach();
  }
  handleNewThread(&serverSocket);

  return 0;
}
