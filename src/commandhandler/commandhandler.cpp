#include <string>
#include <deque>
#include <map>
#include <unistd.h>

#include "commandhandler.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../misc/queries.hpp"

CommandHandlerError::CommandHandlerError(const std::string message){
    this->message = message;
}

CommandHandlerBadArgumentsError::CommandHandlerBadArgumentsError(const std::string command, const std::string expected)
    : CommandHandlerError("Incorrect number of parameters for command "+command+". Expected: "+expected){

}

std::string CommandHandlerError::what(){
    return this->message;
}

CommandHandler::CommandHandler(TCPConnection &conn, SQLiteConnector &dbC) : dbConnector(dbC), connection(conn) {
    addCommandFunction("help", std::bind(&CommandHandler::helpCommand, this, std::placeholders::_1));
    addCommandFunction("register", std::bind(&CommandHandler::registerCommand, this, std::placeholders::_1));
    addCommandFunction("login", std::bind(&CommandHandler::loginCommand, this, std::placeholders::_1));
    addCommandFunction("logoff", std::bind(&CommandHandler::logoffCommand, this, std::placeholders::_1));
    addCommandFunction("list", std::bind(&CommandHandler::listCommand, this, std::placeholders::_1));
    addCommandFunction("grant", std::bind(&CommandHandler::grantCommand, this, std::placeholders::_1));
    addCommandFunction("revoke", std::bind(&CommandHandler::revokeCommand, this, std::placeholders::_1));
    addCommandFunction("shutdown", std::bind(&CommandHandler::shutdownCommand, this, std::placeholders::_1));
    addCommandFunction("exit", std::bind(&CommandHandler::exitCommand, this, std::placeholders::_1));
    addCommandFunction("clientlist", std::bind(&CommandHandler::clientlistCommand, this, std::placeholders::_1));
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

std::string CommandHandler::registerCommand(paramDeque params){
    if (params.size() != 1){
        throw CommandHandlerError("Incorrect number of parameters for command register. Expected: register [name]");
    }

    std::string username = params[0];

    try{
        SQLiteQuery query = SQLiteQuery(INSERT_USER, this->dbConnector);
        query.bindText(1, username);
        query.runOperation();
    }catch(SQLiteQueryError &e){
        throw CommandHandlerError("Error: "+e.what());
    }
    
    return "Registered new user: "+username;
}

std::string CommandHandler::clientlistCommand(paramDeque params){
    if (params.size() != 0){
        throw CommandHandlerError("Incorrect number of parameters for command clientlist. Expected: clientlist");
    }

    std::string resultString = "List of available clients:\n";

    SQLiteQuery query = SQLiteQuery(SELECT_USERS, this->dbConnector);
    auto rows = query.runQuery();
    for (auto row : rows){
        resultString += row.at("username")+"\n";
    }
    

    return resultString;
}

std::string CommandHandler::loginCommand(paramDeque params){
    if (params.size() != 1){
        throw CommandHandlerError("Incorrect number of parameters for command login. Expected: login [name]");
    }

    return "LOGIN";
}

std::string CommandHandler::logoffCommand(paramDeque params){
    if (params.size() != 0){
        throw CommandHandlerError("Incorrect number of parameters for command logoff. Expected: logoff");
    }

    return "LOGOFF";
}

std::string CommandHandler::listCommand(paramDeque params){
    if (params.size() != 0){
        throw CommandHandlerError("Incorrect number of parameters for command list. Expected: list");
    }

    return "LIST";
}

std::string CommandHandler::grantCommand(paramDeque params){
    if (params.size() != 2){
        throw CommandHandlerError("Incorrect number of parameters for command grant. Expected: grant [name] [IP]");
    }

    return "GRANT";
}

std::string CommandHandler::revokeCommand(paramDeque params){
    if (params.size() != 2){
        throw CommandHandlerError("Incorrect number of parameters for command revoke. Expected: revoke [name] [IP]");
    }

    return "REVOKE";
}

std::string CommandHandler::shutdownCommand(paramDeque params){
    if (params.size() != 1){
        throw CommandHandlerError("Incorrect number of parameters for command shutdown. Expected: shutdown [IP]");
    }

    return "SHUTDOWN";
}

std::string CommandHandler::exitCommand(paramDeque params){
    if (params.size() != 0){
        throw CommandHandlerError("Incorrect number of parameters for command exit. Expected: exit");
    }

    return "";
}

std::string CommandHandler::helpCommand(paramDeque params){
    if (params.size() != 0){
        throw CommandHandlerError("Incorrect number of parameters for command help. Expected: help");
    }

    std::string helpMsg = 
                        "- register [name] - add a new client\n"
                        "- clientlist - list of available clients\n"
                        "- login [name] - add current machine as [name] client's machine\n"
                        "- logoff - remove current machine from [name] client's machines\n"
                        "- list - list of available machines and permissions to them for current client\n"
                        "- grant [name] [IP] - grant client [name] permission to shutdown machine [IP]\n"
                        "- revoke [name] [IP] - revoke client's [name] permission to shutdown machine [IP]\n"
                        "- shutdown [IP] - shutdown machine [IP]\n"
                        "- exit - exit from application";

    return helpMsg;
}

CommandHandler::~CommandHandler(){
    
}