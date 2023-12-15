#include "commandparser.hpp"

#include <deque>
#include <iostream>
#include <sstream>

CommandParserError::CommandParserError(const std::string message) {
  this->message = message;
}

char const* CommandParserError::what() { return message.c_str(); }

CommandParser::CommandParser() {}

std::deque<std::string> CommandParser::parseCommand(std::string command) {
  std::deque<std::string> deque;
  std::istringstream iss(command);
  std::string s;
  while (getline(iss, s, ' ')) {
    deque.push_back(s);
  }

  return deque;
}
