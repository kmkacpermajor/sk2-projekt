extern "C" {
#include "../../include/sqlite3.h"
}
#include <iostream>
#include <map>
#include <vector>

#include "../misc/const.hpp"
#include "../misc/queries.hpp"
#include "../sqlitequery/sqlitequery.hpp"
#include "sqliteconnector.hpp"

DatabaseError::DatabaseError(const std::string message) {
  this->message = message;
}

std::string DatabaseError::what() { return this->message; }

SQLiteConnector::SQLiteConnector(std::string dbName) {
  this->dbName = dbName;

  int sqliteStatus = sqlite3_open(this->dbName.c_str(), &(this->db));

  if (sqliteStatus != SQLITE_OK) {
    throw DatabaseError(sqlite3_errmsg(this->db));
  }
}

void SQLiteConnector::initDatabase() {
  SQLiteQuery(ALLOW_FOREIGN_KEYS, this).runOperation();
  try {
    int howManyTables = std::stoi(
        SQLiteQuery(HOW_MANY_TABLES, this).runQuery().at(0).at("count"));
    if (howManyTables != NUM_OF_TABLES) {
      SQLiteQuery("BEGIN", this).runOperation();
      SQLiteQuery(CREATE_USERS, this).runOperation();
      SQLiteQuery(CREATE_MACHINES, this).runOperation();
      SQLiteQuery(CREATE_ALLOWED_SHUTDOWNS, this).runOperation();
      SQLiteQuery(RESET_MACHINES, this).runOperation();
      SQLiteQuery("COMMIT", this).runOperation();
    }
  } catch (SQLiteQueryError &e) {
    SQLiteQuery("ROLLBACK", this).runOperation();
    throw DatabaseError(e.what());
  }
}

sqlite3 *SQLiteConnector::getDatabase() { return this->db; }

SQLiteConnector::~SQLiteConnector() { sqlite3_close(this->db); }