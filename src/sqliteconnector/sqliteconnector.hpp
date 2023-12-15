#pragma once
extern "C" {
#include "../../include/sqlite3.h"
}
#include <map>
#include <string>
#include <vector>

class DatabaseError : public std::exception {
  std::string message;

 public:
  DatabaseError(const std::string message);
  char const *what();
};

class SQLiteConnector {
  std::string dbName;
  sqlite3 *db;

  int howManyTables();

 public:
  SQLiteConnector(std::string dbName);
  void initDatabase();
  sqlite3 *getDatabase();
  ~SQLiteConnector();
};