#include <sstream>
#include <iostream>
#include <deque>

#include "commandparser.hpp"

CommandParserError::CommandParserError(const std::string message)
{
    this->message = message;
}

std::string CommandParserError::what()
{
    return message;
}

CommandParser::CommandParser()
{
}

std::deque<std::string> CommandParser::parseCommand(std::string command)
{
    std::deque<std::string> deque;
    std::istringstream iss(command);
    std::string s;
    while (getline(iss, s, ' '))
    {
        deque.push_back(s);
    }

    return deque;
}

CommandParser::~CommandParser()
{
}
