#ifndef ASCII_CLASH_DBMANAGER_H
#define ASCII_CLASH_DBMANAGER_H
#include <sqlite3.h>
#include <string>

#include "GameLogic/Monsters/CreateMonster.h"
#include "Server/PlayerSession.h"

inline const std::string DBPaths = "Database/DB_Files/";

class DBManager {
public:
    explicit DBManager(const std::string &dbName);

    bool TryRegisterPlayer(const std::string *username, const std::string *password, std::string *err = nullptr) const;

    bool TryCreateMonster(const std::string &name, MonsterType type, PlayerSession *player,
                          std::string *err = nullptr) const;

    void DeleteMonster(int id, PlayerSession *player) const;

    PlayerSession *GetNewPlayerSession(const std::string *username, const std::string *password) const;

    void SavePlayer(const PlayerSession *toSave) const;

    [[nodiscard]] std::vector<std::tuple<std::string, int> > GetLeaderBoard() const;

    void LoadMonstersIntoPlayer(PlayerSession *player) const;

    ~DBManager();

private:
    void InitDB() const;


    void SaveMonster(Monster *toSave, int owner_id) const;


    sqlite3 *db{};
};


#endif
