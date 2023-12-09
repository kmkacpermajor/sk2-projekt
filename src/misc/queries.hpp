#pragma once
#define HOW_MANY_TABLES "SELECT COUNT(*) count FROM sqlite_schema WHERE type ='table' AND name NOT LIKE 'sqlite_%'"

#define CREATE_USERS "CREATE TABLE IF NOT EXISTS users (username TEXT NOT NULL)"

#define CREATE_MACHINES "CREATE TABLE IF NOT EXISTS machines (ip_address TEXT NOT NULL, state INTEGER NOT NULL)"

#define CREATE_ALLOWED_SHUTDOWNS "CREATE TABLE IF NOT EXISTS allowed_shutdowns (user_id INTEGER, machine_id INTEGER, FOREIGN KEY(user_id) REFERENCES users(rowid), FOREIGN KEY(machine_id) REFERENCES machines(rowid))"

#define SELECT_USERS "SELECT username FROM users"

#define SELECT_USER_ID "SELECT rowid FROM users WHERE username = ?"

#define SELECT_MACHINE_ID "SELECT rowid FROM machines WHERE ip_address = ?"

#define SELECT_LAST_ID "SELECT last_insert_rowid() rowid"

#define SELECT_MACHINES_AND_PERMISSIONS "SELECT m.ip_address, m.state, a. FROM machines m LEFT OUTER JOIN allowed_shutdowns a ON m.rowid = a.machine_id WHERE a.user_id = ?"

#define INSERT_USER "INSERT INTO users (username) VALUES (?)"

#define INSERT_MACHINE "INSERT INTO machines (ip_address, file_descriptor, state) VALUES (?, ?, 1)"

#define INSERT_ALLOWED_SHUTDOWN "INSERT INTO allowed_shutdowns (user_id, machine_id) SELECT u.rowid, m.rowid FROM users u INNER JOIN machines m WHERE u.username = ? OR m.ip_address = ?"

#define SET_STATUS "UPDATE machines SET state = ? WHERE ip_address = ?"

#define DELETE_ALLOWED_SHUTDOWN "DELETE FROM allowed_shutdowns WHERE user_id = ? AND machine_id = ?"