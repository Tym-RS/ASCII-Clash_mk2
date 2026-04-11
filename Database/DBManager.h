#ifndef ASCII_CLASH_DBMANAGER_H
#define ASCII_CLASH_DBMANAGER_H
#include <sqlite3.h>
#include <string>

#include "GameLogic/Monsters/Monsters.h"
#include "Server/PlayerSession.h"

inline const std::string DBPaths = "Database/DB_Files/";

class DBManager {
public:
    explicit DBManager(const std::string &dbName);

    bool TryRegisterPlayer(const std::string *username, const std::string *password, std::string *err) const;

    PlayerSession *TryGetPlayer(const std::string *username, const std::string *password) const;

    void SavePlayer() const;

    void SaveMonster() const;

    Monster GetMonsterByID(int id) const;

    void GetScoreboard() const;

    ~DBManager();

private:
    void InitDB() const;

    sqlite3 *db{};
};


#endif
