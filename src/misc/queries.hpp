#pragma once
#define ALLOW_FOREIGN_KEYS "PRAGMA foreign_keys = ON"

#define HOW_MANY_TABLES                                                        \
  "SELECT COUNT(*) count FROM sqlite_schema WHERE type ='table' AND name NOT " \
  "LIKE 'sqlite_%'"

#define CREATE_USERS                                                    \
  "CREATE TABLE IF NOT EXISTS users (user_id INTEGER PRIMARY KEY ASC, " \
  "username "                                                           \
  "TEXT UNIQUE NOT NULL)"

#define CREATE_MACHINES                                                       \
  "CREATE TABLE IF NOT EXISTS machines (machine_id INTEGER PRIMARY KEY ASC, " \
  "ip_address TEXT NOT NULL, "                                                \
  "user_id NOT NULL, file_descriptor NOT NULL, state INTEGER "                \
  "NOT NULL, UNIQUE(ip_address, user_id))"

#define CREATE_ALLOWED_SHUTDOWNS                                             \
  "CREATE TABLE IF NOT EXISTS allowed_shutdowns (rowid INTEGER PRIMARY KEY " \
  "ASC, user_id INTEGER NOT NULL, "                                          \
  "machine_id INTEGER NOT NULL, FOREIGN KEY(user_id) REFERENCES "            \
  "users(rowid), FOREIGN KEY(machine_id) REFERENCES machines(rowid) ON "     \
  "DELETE CASCADE, UNIQUE(user_id, machine_id))"

#define SELECT_USERS "SELECT * FROM users"

#define SELECT_USER "SELECT * FROM users WHERE username = ?"

#define SELECT_MACHINE "SELECT * FROM machines WHERE ip_address = ?"

#define SELECT_LAST_ID "SELECT last_insert_rowid() rowid"

#define SELECT_USER_AND_MACHINE_ID                                             \
  "SELECT u.rowid as user_id, "                                                \
  "m.rowid as machine_id from users u cross join machines m where u.username " \
  "= ? and m.ip_address = ?"

#define SELECT_ALLOWED_SHUTDOWN                                             \
  "SELECT a.rowid FROM allowed_shutdowns a INNER JOIN machines m ON "       \
  "a.machine_id = m.rowid INNER JOIN users u ON a.user_id = u.rowid WHERE " \
  "u.username = ? AND m.ip_address = ?"

// left joins didnt work as expected
#define SELECT_ALLOWED_SHUTDOWNS                                             \
  "SELECT *, 0 permission FROM (SELECT m.ip_address, m.state FROM "          \
  "allowed_shutdowns a INNER JOIN machines m ON a.machine_id = m.rowid "     \
  "EXCEPT SELECT m.ip_address, m.state FROM allowed_shutdowns a INNER JOIN " \
  "machines m ON a.machine_id = m.rowid WHERE a.user_id = ?1) UNION SELECT " \
  "m.ip_address, m.state, 1 FROM allowed_shutdowns a INNER JOIN machines m " \
  "ON a.machine_id = m.rowid WHERE a.user_id = ?1"

#define SELECT_MACHINE_OWNER                                             \
  "SELECT u.rowid id, u.username FROM users u INNER JOIN machines m ON " \
  "u.rowid = m.user_id WHERE m.ip_address = ?"

#define SELECT_MACHINES_AND_PERMISSIONS                               \
  "SELECT m.ip_address, m.state, a. FROM machines m LEFT OUTER JOIN " \
  "allowed_shutdowns a ON m.rowid = a.machine_id WHERE a.user_id = ?"

#define INSERT_USER "INSERT INTO users (username) VALUES (?)"

#define INSERT_MACHINE                                                         \
  "INSERT INTO machines (ip_address, file_descriptor, user_id, state) VALUES " \
  "(?, ?, ?, 1)"

#define INSERT_ALLOWED_SHUTDOWN \
  "INSERT INTO allowed_shutdowns (user_id, machine_id) VALUES (?, ?)"

#define SET_STATUS "UPDATE machines SET state = ? WHERE ip_address = ?"

#define RESET_MACHINES "UPDATE machines SET state = 0, file_descriptor = -1"

#define SET_FD "UPDATE machines SET file_descriptor = ? WHERE ip_address = ?"

#define FINALIZE_MACHINE \
  "UPDATE machines SET file_descriptor = -1, state = 0 WHERE ip_address = ?"

#define DELETE_MACHINES_ALLOWED_SHUTDOWNS                                      \
  "DELETE FROM allowed_shutdowns WHERE rowid IN (SELECT a.rowid FROM "         \
  "allowed_shutdowns a INNER JOIN machines m ON a.machine_id = m.rowid WHERE " \
  "m.ip_address = ?)"

#define DELETE_ALLOWED_SHUTDOWN                                                \
  "DELETE FROM allowed_shutdowns WHERE rowid IN (SELECT a.rowid FROM "         \
  "allowed_shutdowns a INNER JOIN machines m ON a.machine_id = m.rowid INNER " \
  "JOIN users u ON a.user_id = u.rowid WHERE u.username = ? AND m.ip_address " \
  "= ?)"

#define DELETE_MACHINE "DELETE FROM machines WHERE ip_address = ?"