#pragma once
extern "C" {
#include <sqlite3.h>
}
#include <string>
#include <vector>
#include <map>

class DatabaseError : public std::exception {
    std::string message;
    public:
    DatabaseError(const std::string message);
    std::string what();
};

class SQLiteConnector{
    std::string dbName;
    sqlite3* db;

    public:
        SQLiteConnector(std::string dbName);
        void createTables();
        void runOperation(std::string sql);
        void initDatabase();
        sqlite3_stmt* prepareQuery(std::string sql);
        std::vector<std::map<std::string, std::string>> getQueryResults(sqlite3_stmt* statement);
        void finalizeQuery(sqlite3_stmt* statement);
        std::vector<std::map<std::string, std::string>> runQuery(std::string sql);
        int howManyTables();
        sqlite3* getDatabase();
        ~SQLiteConnector();
};