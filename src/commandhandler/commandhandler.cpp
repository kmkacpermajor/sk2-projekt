#include <string>
#include <deque>
#include <map>

#include "commandhandler.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"

CommandHandlerError::CommandHandlerError(const std::string message){
    this->message = message;
}

std::string CommandHandlerError::what(){
    return message;
}

CommandHandler::CommandHandler(TCPConnection &conn, SQLiteConnector &dbC) : dbConnector(dbC), connection(conn) {
    addCommandFunction("help", std::bind(&CommandHandler::helpCommand, this, std::placeholders::_1));
    // register [name]
    // login [name]
    // logoff [name]
    // list
    // grant [name] [IP]
    // revoke [name] [IP]
    // shutdown [IP]
    // exit
}

void CommandHandler::addCommandFunction(std::string command, const commandFunction& func) {
    commandFunctions[command] = func;
}

std::string CommandHandler::handleCommand(std::string command, paramDeque params) {
    auto it = commandFunctions.find(command);
    if (it != commandFunctions.end()) {
        return it->second(params);
    } else {
        throw CommandHandlerError("Unknown command: " + command);
    }
}

std::string CommandHandler::helpCommand(paramDeque params){
    if (params.size() != 0){
        throw CommandHandlerError("Incorrect number of parameters for command help");
    }

    return "HELP";
}

// std::string CommandHandler::helpCommand(paramDeque params){
//     if (params.size() != 0){
//         throw CommandHandlerError("Incorrect number of parameters for command help");
//     }

//     return "HELP";
// }

CommandHandler::~CommandHandler(){
    
}