#pragma once
extern "C" {
#include "../../include/sqlite3.h"
}
#include <string>

#include "../sqliteconnector/sqliteconnector.hpp"

class SQLiteQueryError : public std::exception {
  std::string message;
  int errNo;

 public:
  SQLiteQueryError(const std::string message, const int errNo);
  char const *what();
  int what_errno();
};

class SQLiteQuery {
  sqlite3_stmt *statement;
  sqlite3 *db;

  void checkForError(int sqliteStatus);

 public:
  SQLiteQuery(std::string sql, SQLiteConnector *dbConnector);
  SQLiteQuery(std::string sql, sqlite3 *db);
  int runOperation();
  int getLastID();
  SQLiteQuery &bindText(int index, std::string value);
  SQLiteQuery &bindInt(int index, int value);
  std::vector<std::map<std::string, std::string>> runQuery();
  ~SQLiteQuery();
};