#include "DBManager.h"
#include <filesystem>
#include <iostream>
#include <regex>


DBManager::DBManager(const std::string &dbName) {
    const std::string path = DBPaths + dbName + ".db";
    const bool isNewDB = std::filesystem::exists(path) == false;
    sqlite3_open(path.c_str(), &db);
    if (isNewDB) InitDB();
}

bool DBManager::TryRegisterPlayer(const std::string *username, const std::string *password,
                                  std::string *err = nullptr) const {
    if (!std::regex_match(*username, Config::Players::usernameRegex)) {
        if (err) *err = "Username must be 1-15 characters long, no spaces, only letters numbers _ and -";
        return false;
    }

    if (!std::regex_search(*password, std::regex("[A-Za-z]")) ||
        !std::regex_search(*password, std::regex("[0-9]"))) {
        if (err) *err = "Password must contain letters and numbers.";
        return false;
    }


    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT username FROM users WHERE username = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username->c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (err) *err = "Username is already Taken";
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    sqlite3_prepare_v2(db, "INSERT INTO users (username, password) VALUES (?, ?);", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username->c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, std::hash<std::string>{}(*password));
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        if (err) *err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool DBManager::TryCreateMonster(const std::string &name, const MonsterType type, PlayerSession *player,
                                 std::string *err) const {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM monsters WHERE owner_id = ? and name = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, player->PlayerID);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (err) *err = "Name already exists in the team.";
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    sqlite3_prepare(db, "SELECT COALESCE(MAX(id), 0) + 1 FROM monsters;", -1, &stmt, nullptr);
    int nextId = 1;
    if (sqlite3_step(stmt) == SQLITE_ROW) nextId = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    LoadMonstersIntoPlayer(player);
    for (int i = 0; i < Config::Players::TeamSize; i++) {
        if (player->Monsters[i]) continue;
        player->Monsters[i] = CreateTypedMonster(name, nextId, type);
        SaveMonster(player->Monsters[i], player->PlayerID);
        return true;
    }
    if (err) *err = "Team already full.";
    return false;
}

void DBManager::DeleteMonster(const int id, PlayerSession *player) const {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM monsters WHERE id = ? AND owner_id = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, player->PlayerID);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    LoadMonstersIntoPlayer(player);
}

PlayerSession *DBManager::GetNewPlayerSession(const std::string *username, const std::string *password) const {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM users WHERE username = ? and password = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username->c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, std::hash<std::string>{}(*password));
    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (id == -1) return nullptr;
    auto *player = new PlayerSession(id, *username);
    LoadMonstersIntoPlayer(player);
    return player;
}

void DBManager::SavePlayer(const PlayerSession *toSave) const {
    const std::string cmd = "UPDATE users SET score = " + std::to_string(toSave->Score) + " WHERE id = " +
                            std::to_string(toSave->PlayerID) + ";";
    sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, nullptr);
    for (Monster *mon: toSave->Monsters) if (mon) SaveMonster(mon, toSave->PlayerID);
}

std::vector<std::tuple<std::string, int> > DBManager::GetLeaderBoard() const {
    sqlite3_stmt *stmt;
    std::vector<std::tuple<std::string, int> > result;

    std::string cmd = "SELECT username, score FROM users ORDER BY score DESC LIMIT ";
    cmd += std::to_string(Config::Server::LeaderboardSize) + ";";
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        int score = sqlite3_column_int(stmt, 1);
        result.emplace_back(username, score);
    }

    sqlite3_finalize(stmt);
    return result;
}

void DBManager::LoadMonstersIntoPlayer(PlayerSession *player) const {
    sqlite3_stmt *stmt;
    std::string cmd = "SELECT id, name, type ";
    for (const auto &stat: StatStringMap | std::views::keys) {
        cmd += ", " + stat;
    }
    cmd += " FROM monsters WHERE owner_id = " + std::to_string(player->PlayerID) + ";";
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);

    bool done = false;
    for (int i = 0; i < Config::Players::TeamSize; i++) {
        if (player->Monsters[i]) delete player->Monsters[i];
        if (!done) done = sqlite3_step(stmt) != SQLITE_ROW;
        if (done) {
            player->Monsters[i] = nullptr;
            continue;
        }
        const int id = sqlite3_column_int(stmt, 0);
        const std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const std::string typeS = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        std::array<int, static_cast<int>(Stat::COUNT)> initValues{};
        int stat_i = 3;
        for (const auto &stat: StatStringMap | std::views::values) {
            initValues.at(static_cast<int>(stat)) = sqlite3_column_int(stmt, stat_i);
            stat_i++;
        }
        player->Monsters[i] = CreateTypedMonster(name, id, MonsterTypeStringMap.at(typeS), new StatDict(initValues));
    }
    sqlite3_finalize(stmt);
}

void DBManager::SaveMonster(Monster *toSave, const int owner_id) const {
    if (!toSave->IsAlive()) {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "DELETE FROM monsters WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, toSave->ID);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return;
    }

    std::string cmd = "INSERT OR REPLACE INTO monsters (id, owner_id, name, type";
    for (const auto &stat: StatStringMap | std::views::keys)
        cmd += "," + stat;
    cmd += ") VALUES (?, ?, ?, ?";
    for (int i = 0; i < std::size(StatInfos); i++) cmd += ", ?";
    cmd += ");";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);

    sqlite3_bind_int(stmt, 1, toSave->ID);
    sqlite3_bind_int(stmt, 2, owner_id);
    sqlite3_bind_text(stmt, 3, toSave->Name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, MonsterDescriptions.at(toSave->Type).TypeAsString.c_str(), -1, SQLITE_STATIC);
    int i = 5;
    for (const auto &stat: StatStringMap | std::views::values) {
        sqlite3_bind_int(stmt, i, toSave->GetStatDict()->Get(stat));
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
                 "password INTEGER NOT NULL,"
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
