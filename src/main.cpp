#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <deque>
#include <thread>

#include "misc/stringTrim.hpp"
#include "misc/types.hpp"
#include "misc/const.hpp"
#include "tcpconnection/tcpconnection.hpp"
#include "sqliteconnector/sqliteconnector.hpp"
#include "commandhandler/commandhandler.hpp"
#include "commandparser/commandparser.hpp"
#include "authverifier/authverifier.hpp"
#include "sqlitequery/sqlitequery.hpp"
#include "misc/queries.hpp"

std::mutex m;

void handleNewServerConnection(int serverFD)
{
  auto connection = TCPConnection(serverFD, m);
  auto dbConnector = SQLiteConnector(DB_NAME);
  auto commandParser = CommandParser();
  auto commandHandler = CommandHandler(connection, dbConnector);

  try{
    try{
      auto ownerRow = SQLiteQuery(SELECT_MACHINE_OWNER, &dbConnector).bindText(1, connection.getIPAddress())->runQuery().at(0);
      SQLiteQuery(SET_STATUS, &dbConnector).bindInt(1, 1)->bindText(2, connection.getIPAddress())->runOperation();
      connection.setCurrentUser(ownerRow.at("username"), std::stoi(ownerRow.at("id")));
    }catch (std::out_of_range& e){
      std::cout << "No user for current machine. Not relogging" << std::endl;
    }
  }catch(SQLiteQueryError& e){
    std::cout << "Error occurred when trying to relogin: " << e.what() << std::endl;
  }

  if (connection.getCurrentUser() == ""){
    connection.sendMessage("Połączono z serwerem");
  }else{
    connection.sendMessage("Połączono z serwerem jako użytkownik "+connection.getCurrentUser());
  }

  while (1)
  {
    std::string message;
    std::string response;
    try
    {
      message = connection.getMessage();
    }
    catch (ConnectionError &e)
    {
      std::cout << "Error occurred when getting new message: " << e.what() << std::endl;
      break;
    }

    try
    {
      if (message.empty()){
        response = "";
      }else{
        paramDeque params = commandParser.parseCommand(message);
        
        std::string command = params.front();
        params.pop_front();

        AuthVerifier(connection, dbConnector).verifyCommand(command, params);

        response = commandHandler.handleCommand(command, params);
      }
    }
    catch (CommandHandlerError &e)
    {
      response = e.what();
      std::cout << "Error occurred when handling command: " << message << ": " << e.what() << std::endl;
    }
    catch (CommandParserError &e)
    {
      response = e.what();
      std::cout << "Error occurred when parsing command: " << message << ": " << e.what() << std::endl;
    }catch(AuthVerifierError& e)
    {
      response = e.what();
      std::cout << "Error occurred when authorizing command: " << message << ": " << e.what() << std::endl;
    }

    if (response.length() == 0)
    {
      return;
    }

    try
    {
      connection.sendMessage(trim(response));
    }
    catch (ConnectionError &e)
    {
      std::cout << "Error occurred when sending response: " << e.what() << std::endl;
    }
    catch (ConnectionEndedException &e){
      return;
    }
  }
}

void *handleNewThread(void *arg)
{
  int *serverFD = (int *)arg;

  try
  {
    while (1)
    {
      handleNewServerConnection(*serverFD);
    }
  }
  catch (ConnectionError &e)
  {
    std::cout << "Error occurred when connecting: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  catch (DatabaseError &e)
  {
    std::cout << "Failed to create database: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  return 0;
}

int main()
{
  int serverSocket;
  struct sockaddr_in serverAddress;

  try
  {
    SQLiteConnector initSQLiteConnector = SQLiteConnector(DB_NAME);
    initSQLiteConnector.initDatabase();
  }
  catch (DatabaseError &e)
  {
    std::cout << "Error occurred when initializing database: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));
  bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  if (listen(serverSocket, 50) == 0)
    printf("Listening on port %d\n", PORT);
  else
  {
    perror("Cannot listen");
  }

  for (int i = 0; i < THREAD_COUNT; i++)
  {
    std::thread thread(handleNewThread, &serverSocket);
    thread.detach();
  }
  handleNewThread(&serverSocket);

  return 0;
}
