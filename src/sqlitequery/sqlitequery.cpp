extern "C"
{
#include "../../include/sqlite3.h"
}
#include <string>
#include <vector>
#include <map>

#include "sqlitequery.hpp"
#include "../sqliteconnector/sqliteconnector.hpp"

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

void SQLiteQuery::checkForError(int sqliteStatus)
{
    if (sqliteStatus != SQLITE_OK && sqliteStatus != SQLITE_DONE)
    {
        std::string stringErrorMessage(sqlite3_errmsg(this->db));
        throw SQLiteQueryError(stringErrorMessage);
    }
}

void SQLiteQuery::runOperation()
{
    int sqliteStatus = sqlite3_step(this->statement);
    checkForError(sqliteStatus);
}

void SQLiteQuery::bindText(int index, std::string text)
{
    int sqliteStatus = sqlite3_bind_text(this->statement, index, text.c_str(), -1, NULL);
    checkForError(sqliteStatus);
}

void SQLiteQuery::bindInt(int index, int text)
{
    int sqliteStatus = sqlite3_bind_int(this->statement, index, text);
    checkForError(sqliteStatus);
}

std::vector<std::map<std::string, std::string>> SQLiteQuery::runQuery()
{
    std::vector<std::map<std::string, std::string>> vectorOfMaps;
    int sqliteStatus;
    int col = 0;

    while ((sqliteStatus = sqlite3_step(this->statement)) == SQLITE_ROW)
    {
        std::map<std::string, std::string> row;
        char *result = (char *)sqlite3_column_text(this->statement, col);
        char *colName = (char *)sqlite3_column_name(this->statement, col);
        std::string resultString(result);
        std::string colNameString(colName);
        row[colNameString] = resultString;
        vectorOfMaps.push_back(row);
        col++;
    }

    checkForError(sqliteStatus);

    return vectorOfMaps;
}

SQLiteQuery::~SQLiteQuery()
{
    sqlite3_finalize(this->statement);
}