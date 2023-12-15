#pragma once
#include <deque>
#include <string>

class CommandParserError : public std::exception {
  std::string message;

 public:
  CommandParserError(const std::string message);
  char const* what();
};

class CommandParser {
 public:
  CommandParser();
  std::deque<std::string> parseCommand(std::string command);
};