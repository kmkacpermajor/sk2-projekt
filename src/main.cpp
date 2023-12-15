#include <iostream>
#include <thread>

#include "agentconnection/agentconnection.hpp"
#include "misc/const.hpp"
#include "misc/types.hpp"
#include "networking/networking.hpp"
#include "sqliteconnector/sqliteconnector.hpp"

std::mutex m;

void initDatabase() {
  try {
    SQLiteConnector initSQLiteConnector = SQLiteConnector(DB_NAME);
    initSQLiteConnector.initDatabase();
  } catch (DatabaseError &e) {
    std::cout << "Error occurred when initializing database: " << e.what()
              << std::endl;
    exit(EXIT_FAILURE);
  }
}

void handleNewThread(void *arg) {
  int serverFD = *(int *)arg;

  try {
    while (1) {
      AgentConnection agent(serverFD, m);
      while (1) {
        try {
          std::string message = agent.getConnection().getMessage();

          std::string response = agent.prepareResponse(message);

          if (response.length() == 0) {
            break;
          }

          agent.getConnection().sendMessage(response);
        } catch (ConnectionEndedException &e) {
          break;
        }
      }
    }
  } catch (ConnectionError &e) {
    std::cout << "Error occurred when connecting: " << e.what() << std::endl;
  } catch (DatabaseError &e) {
    std::cout << "Failed to connect to database: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main() {
  initDatabase();

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
