#include <string>
#include <deque>

class CommandParserError : public std::exception
{
    std::string message;

public:
    CommandParserError(const std::string message);
    std::string what();
};

class CommandParser
{
public:
    CommandParser();
    std::deque<std::string> parseCommand(std::string command);
    ~CommandParser();
};