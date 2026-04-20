#ifndef ASCII_CLASH_PLAYERSESSION_H
#define ASCII_CLASH_PLAYERSESSION_H

#include "nlohmann/json.hpp"
#include "../Config.h"
#include "../GameLogic/Monsters/Monsterbase.h"
#include "GameLogic/Fighting/SinglePlayer/SingleplayerRun.h"


class PlayerSession {
public:
    explicit PlayerSession(int playerID, std::string username);

    [[nodiscard]] bool IsActive() const;

    void UpdateLastActivity();

    [[nodiscard]] nlohmann::json GetMonsterJson(int monsterID = -1) const;

    bool TryLevelMonster(int id, nlohmann::json data, std::string *err) const;

    bool TryStartNewRun(std::string *err);

    int Score = 0;
    bool AutoFight = false;
    const int PlayerID;
    const std::string SessionID, Username;
    std::array<Monster *, Config::Players::TeamSize> Monsters;

    ~PlayerSession();

private:
    [[nodiscard]] Monster *GetMonsterByID(int id) const;

    SingleplayerRun *currentRun;
    long lastActivity;
};

#endif
