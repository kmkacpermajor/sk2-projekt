#pragma once
extern "C"
{
#include "../../include/sqlite3.h"
}
#include <string>
#include <vector>
#include <map>

class DatabaseError : public std::exception
{
    std::string message;

public:
    DatabaseError(const std::string message);
    std::string what();
};

class SQLiteConnector
{
    std::string dbName;
    sqlite3 *db;

public:
    SQLiteConnector(std::string dbName);
    void initDatabase();
    int howManyTables();
    sqlite3 *getDatabase();
    ~SQLiteConnector();
};