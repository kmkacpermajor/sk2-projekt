#include <string>
#include <deque>

#include "authverifier.hpp"
#include "../tcpconnection/tcpconnection.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "../misc/stringTrim.hpp"

AuthVerifierError::AuthVerifierError(const std::string message){
    this->message = message;
}

std::string AuthVerifierError::what() {
  return this->message;
}

AuthVerifier::AuthVerifier(TCPConnection &conn, SQLiteConnector &dbC) : connection(conn), dbConnector(dbC)
{
}

bool AuthVerifier::checkIfLoggedIn(){
    if(connection.getCurrentUser() == ""){
        return false;
    }
    return true;
}

void AuthVerifier::verifyCommand(std::string command, std::deque<std::string> params) {
    if(isStringInVector(commandsLoggedIn, command) && !checkIfLoggedIn()){
        throw AuthVerifierError("This command can be used only when logged in");
    }

    if(isStringInVector(commandsNotLoggedIn, command) && checkIfLoggedIn()){
        throw AuthVerifierError("This command can be used only when logged off");
    }
}