#ifndef ASCII_CLASH_PLAYERSESSION_H
#define ASCII_CLASH_PLAYERSESSION_H

#include "nlohmann/json.hpp"
#include "../Config.h"
#include "../GameLogic/Monsters/Monsters.h"


class PlayerSession {
public:
    explicit PlayerSession(int playerID);

    [[nodiscard]] bool IsActive() const;

    void UpdateLastActivity();

    [[nodiscard]] nlohmann::json GetMonsterJson(int monsterID) const;

    bool TryLevelMonster(int id, nlohmann::json data, std::string *err) const;

    const int PlayerID;
    const std::string ID;
    Monster *Monsters[Config::Players::TeamSize];

    ~PlayerSession();

private:
    [[nodiscard]] Monster *GetMonsterByID(int id) const;

    long lastActivity;
};

#endif
