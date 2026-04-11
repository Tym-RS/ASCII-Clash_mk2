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

    PlayerSession *GetNewPlayerSession(const std::string *username, const std::string *password) const;

    void SavePlayer(const PlayerSession *toSave) const;

    void GetScoreboard() const;

    ~DBManager();

private:
    void InitDB() const;

    void FillPlayerMonsters(PlayerSession *player) const;

    void SaveMonster(Monster *toSave, const PlayerSession *owner) const;


    sqlite3 *db{};
};


#endif
