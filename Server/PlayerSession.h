#ifndef ASCII_CLASH_PLAYERSESSION_H
#define ASCII_CLASH_PLAYERSESSION_H

#include "nlohmann/json.hpp"
#include "../Config.h"
#include "../GameLogic/Monsters/Monsters.h"


class PlayerSession {
public:
    explicit PlayerSession(int playerID, std::string sessionID);

    [[nodiscard]] bool IsActive() const;

    void UpdateLastActivity();

    [[nodiscard]] nlohmann::json GetMonsterJson(const std::string &ID = "") const;

    bool TryLevelMonster(const std::string &id, nlohmann::json data, std::string *err) const;

    const int PlayerID;
    const std::string SessionID;
    Monster *Monsters[Config::Players::TeamSize];

    ~PlayerSession();

private:
    Monster *GetMonsterByID(const std::string &id) const;

    double lastActivity;
};

#endif
