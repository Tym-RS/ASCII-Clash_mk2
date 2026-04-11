#include "DBManager.h"

#include <filesystem>

DBManager::DBManager(const std::string &dbName) {
    const std::string path = DBPaths + dbName + ".db";
    const bool isNewDB = std::filesystem::exists(path) == false;
    sqlite3_open(path.c_str(), &db);
    if (isNewDB) InitDB();
}

bool DBManager::TryRegisterPlayer(const std::string *username, const std::string *password,
                                  std::string *err = nullptr) const {
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, "SELECT username FROM users WHERE username = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username->c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (err) *err = "Username is already Taken";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_prepare_v2(db, "INSERT INTO users (username, password) VALUES (?, ?);", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username->c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password->c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (err) *err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        return false;
    }
    return true;
}

PlayerSession *DBManager::TryGetPlayer(const std::string *username, const std::string *password) const {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id FROM users WHERE username = ? and password = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username->c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password->c_str(), -1, SQLITE_STATIC);
    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return id == -1 ? nullptr : new PlayerSession(id);
}

DBManager::~DBManager() {
    sqlite3_close(db);
}


void DBManager::InitDB() const {
    // Create users table
    sqlite3_exec(db, "CREATE TABLE users ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "username TEXT NOT NULL UNIQUE,"
                 "password TEXT NOT NULL,"
                 "score INTEGER DEFAULT 0 NOT NULL" //,"
                 //"sg_run_id INTEGER UNIQUE"
                 ");", nullptr, nullptr, nullptr);

    // Create monsters Table
    std::string cmd = "CREATE TABLE monsters ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "owner_id INTEGER,"
            "FOREIGN KEY (owner_id) REFERENCES users(id) NOT NULL,"
            "name TEXT NOT NULL,";
    for (const auto &info: StatInfos)
        cmd += std::string(info.AsString) + " INTEGER NOT NULL DEFAULT" + std::to_string(
            info.DefaultValue) + ",";
    cmd.pop_back();
    cmd += ");";

    sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, nullptr);
}
