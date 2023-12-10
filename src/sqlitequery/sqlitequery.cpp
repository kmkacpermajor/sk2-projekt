extern "C"
{
#include "../../include/sqlite3.h"
}
#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "sqlitequery.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"
#include "../misc/queries.hpp"

SQLiteQueryError::SQLiteQueryError(const std::string message)
{
    this->message = message;
}

std::string SQLiteQueryError::what()
{
    return this->message;
}

SQLiteQuery::SQLiteQuery(std::string sql, SQLiteConnector *dbConnector)
{
    this->db = dbConnector->getDatabase();
    int sqliteStatus = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &this->statement, NULL);
    checkForError(sqliteStatus);
}

SQLiteQuery::SQLiteQuery(std::string sql, sqlite3 *db)
{
    this->db = db;
    int sqliteStatus = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &this->statement, NULL);
    checkForError(sqliteStatus);
}

void SQLiteQuery::checkForError(int sqliteStatus)
{
    if (sqliteStatus != SQLITE_OK && sqliteStatus != SQLITE_DONE)
    {
        std::string stringErrorMessage(sqlite3_errmsg(this->db));
        throw SQLiteQueryError(stringErrorMessage);
    }
}

int SQLiteQuery::getLastId()
{
    return std::stoi(SQLiteQuery(SELECT_LAST_ID, db).runQuery().at(0).at("rowid"));
}

int SQLiteQuery::runOperation()
{
    auto out = sqlite3_expanded_sql(this->statement);
    std::cout << "Running SQL: " << out << std::endl;
    int sqliteStatus = sqlite3_step(this->statement);
    checkForError(sqliteStatus);
    return getLastId(); // returns last id (id updates after insert, not update)
}

SQLiteQuery* SQLiteQuery::bindText(int index, std::string text)
{   
    int sqliteStatus = sqlite3_bind_text(this->statement, index, text.c_str(), text.length(), SQLITE_TRANSIENT);
    checkForError(sqliteStatus);

    return this;
}

SQLiteQuery* SQLiteQuery::bindInt(int index, int text)
{
    int sqliteStatus = sqlite3_bind_int(this->statement, index, text);
    checkForError(sqliteStatus);

    return this;
}

std::vector<std::map<std::string, std::string>> SQLiteQuery::runQuery()
{
    auto out = sqlite3_expanded_sql(this->statement);
    std::cout << "Running SQL: " << out << std::endl;
    std::vector<std::map<std::string, std::string>> vectorOfMaps;
    int sqliteStatus;
    int colCount = sqlite3_column_count(this->statement);

    while ((sqliteStatus = sqlite3_step(this->statement)) == SQLITE_ROW)
    {
        std::map<std::string, std::string> row;
        for(int col = 0; col < colCount; col++){
            char *result = (char *)sqlite3_column_text(this->statement, col);
            char *colName = (char *)sqlite3_column_name(this->statement, col);
            std::string resultString(result);
            std::string colNameString(colName);
            row[colNameString] = resultString;
        }
        vectorOfMaps.push_back(row);
    }

    checkForError(sqliteStatus);

    return vectorOfMaps;
}

SQLiteQuery::~SQLiteQuery()
{
    sqlite3_finalize(this->statement);
}