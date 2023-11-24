#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

#include "misc/const.hpp"
#include "tcpconnection/tcpconnection.hpp"
#include "sqliteconnector/sqliteconnector.hpp"
#include "commandhandler/commandhandler.hpp"
#include "commandparser/commandparser.hpp"
#include <deque>


void handleNewServerConnection(int serverFD){
  auto connection = TCPConnection(serverFD);
  auto dbConnector = SQLiteConnector(DB_NAME);
  auto commandParser = CommandParser();
  auto commandHandler = CommandHandler(connection, dbConnector);
  connection.sendMessage("Połączono z serwerem");

  while(1){
    std::string message;
    std::string response;
    try{      
      message = connection.getMessage();
    }catch(ConnectionError& e){
      std::cout << "Error occurred when getting new message: " << e.what() << std::endl;
      break;
    }

    try{
      std::deque<std::string> params = commandParser.parseCommand(message);
      if (params.empty()){
        throw CommandParserError("Empty command given");
      }
      std::string command = params.front();
      params.pop_front();


    }catch(CommandHandlerError& e){
      response = e.what();
      std::cout << "Error occurred when handling command: " << message << ": " << e.what() << std::endl;
    }catch(CommandParserError& e){
      response = e.what();
      std::cout << "Error occurred when handling command: " << message << ": " << e.what() << std::endl;
    }

    try{      
      connection.sendMessage(response);
    }catch(ConnectionError& e){
      std::cout << "Error occurred when sending response: " << e.what() << std::endl;
    }
  }
}

void* handleNewThread(void* arg) {
  int* serverFD = (int*)arg;
  
  try{
    while(1) {
      handleNewServerConnection(*serverFD);     
    }
  } catch (ConnectionError& e){
    std::cout << "Error occurred when connecting: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (DatabaseError &e){
    std::cout << "Failed to create database: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  return 0;
}

int main(){
  int serverSocket;
  struct sockaddr_in serverAddress;
  pthread_t tid;

  try{
    SQLiteConnector initSQLiteConnector = SQLiteConnector(DB_NAME);
    initSQLiteConnector.initDatabase();
  } catch (DatabaseError& e){
    std::cout << "Error occurred when initializing database: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } 
  

  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));
  bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

  if(listen(serverSocket,50)==0)
    printf("Listening on port %d\n", PORT);
  else{
    perror("Cannot listen");
  }
  
  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_create(&tid, NULL, handleNewThread, &serverSocket);
    pthread_detach(tid);
  }
  handleNewThread(&serverSocket);

  return 0;
}
