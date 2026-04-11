#include "DBManager.h"
#include <filesystem>
#include <array>

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
    sqlite3_finalize(stmt);
    return true;
}

PlayerSession *DBManager::GetNewPlayerSession(const std::string *username, const std::string *password) const {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM users WHERE username = ? and password = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username->c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password->c_str(), -1, SQLITE_STATIC);
    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (id == -1) return nullptr;
    auto *player = new PlayerSession(id);
    FillPlayerMonsters(player);
    return player;
}

void DBManager::SavePlayer(const PlayerSession *toSave) const {
    const std::string cmd = "UPDATE users SET score = " + std::to_string(toSave->Score) + " WHERE id = " +
                            std::to_string(toSave->PlayerID) + ";";
    sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, nullptr);
    for (Monster *mon: toSave->Monsters) if (mon) SaveMonster(mon, toSave);
}

void DBManager::FillPlayerMonsters(PlayerSession *player) const {
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT id, name, type ";
    for (const auto &stat: StatStringMap | std::views::keys) {
        cmd += ", " + stat;
    }
    cmd += " FROM monsters WHERE owner_id = " + std::to_string(player->PlayerID) + ";";
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const int id = sqlite3_column_int(stmt, 0);
        const std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const std::string type = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        std::array<int, static_cast<int>(Stat::COUNT)> initValues{};
        int stat_i = 3;
        for (const auto &stat: StatStringMap | std::views::values) {
            initValues.at(static_cast<int>(stat)) = sqlite3_column_int(stmt, stat_i);
            stat_i++;
        }
        player->Monsters[i] = new Monster(name, id, MonsterTypeStringMap.at(type), StatDict(initValues));
        i++;
    }


    sqlite3_finalize(stmt);
}


void DBManager::SaveMonster(Monster *toSave, const PlayerSession *owner) const {
    std::string cmd = "INSERT OR REPLACE INTO monsters (id, owner_id, name, type";
    for (const auto &stat: StatStringMap | std::views::keys)
        cmd += "," + stat;
    cmd += ") VALUES (?, ?, ?, ?";
    for (int i = 0; i < std::size(StatInfos); i++) cmd += ", ?";
    cmd += ");";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, toSave->ID);
    sqlite3_bind_int(stmt, 2, owner->PlayerID);
    sqlite3_bind_text(stmt, 3, toSave->Name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, MonsterDescriptions.at(toSave->Type).TypeAsString.c_str(), -1, SQLITE_STATIC);
    int i = 5;
    for (const auto &stat: StatStringMap | std::views::values) {
        sqlite3_bind_int(stmt, i, toSave->GetStatDict().Get(stat));
        i++;
    }
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
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
            "owner_id INTEGER NOT NULL,"
            "name TEXT NOT NULL,"
            "type TEXT NOT NULL,";
    for (const auto &info: StatInfos)
        cmd += std::string(info.AsString) + " INTEGER NOT NULL DEFAULT " + std::to_string(
            info.DefaultValue) + ",";
    cmd += " FOREIGN KEY (owner_id) REFERENCES users(id));";

    sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, nullptr);
}
