#pragma once
extern "C" {
#include "../../include/sqlite3.h"
}
#include <string>

#include "../sqliteconnector/sqliteconnector.hpp"

class SQLiteQueryError : public std::exception {
  std::string message;

 public:
  SQLiteQueryError(const std::string message);
  std::string what();
};

class SQLiteQuery {
  sqlite3_stmt *statement;
  sqlite3 *db;

 public:
  SQLiteQuery(std::string sql, SQLiteConnector *dbConnector);
  SQLiteQuery(std::string sql, sqlite3 *db);
  int runOperation();
  int getLastId();
  void checkForError(int sqliteStatus);
  SQLiteQuery *bindText(int index, std::string value);
  SQLiteQuery *bindInt(int index, int value);
  std::vector<std::map<std::string, std::string>> runQuery();
  ~SQLiteQuery();
};