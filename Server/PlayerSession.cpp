#include "PlayerSession.h"

#include <utility>

using json = nlohmann::json;

inline double Now() {
    return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}


PlayerSession::PlayerSession(const int playerID, std::string sessionID) : PlayerID(playerID),
                                                                          SessionID(std::move(sessionID)) {
    lastActivity = Now();
    Monsters[0] = new Human("George", "AFDSGFKHSF");
    Monsters[1] = new Human("Peter", "ADFFGSFG44");
    Monsters[2] = new Orc("Murkk", "SGSFSHDFEF");
    Monsters[3] = new Orc("Gurk", "safasfe");
    Monsters[4] = nullptr;
}

bool PlayerSession::IsActive() const {
    return lastActivity + Config::Server::SessionTimeout > Now();
}

void PlayerSession::UpdateLastActivity() {
    lastActivity = Now();
}

json PlayerSession::GetMonsterJson(const std::string &ID) const {
    json arr = json::array();
    const bool selectAll = ID.empty();
    for (Monster *monster: Monsters) {
        if (!selectAll && (!monster || monster->ID != ID)) continue;
        if (monster)
            arr.push_back({
                {"id", monster->ID},
                {"name", monster->Name},
                {"type", MonStrings.at(monster->Type).TypeName},
                {"level", monster->GetStatDict().Get(Stat::Level)},
                {"exp", monster->GetStatDict().Get(Stat::Exp)},
                {"skillpoints", monster->GetStatDict().Get(Stat::SkillPoints)},
                {"health", monster->GetStatDict().Get(Stat::Health)},
                {"offense", monster->GetStatDict().Get(Stat::Offense)},
                {"damage", monster->GetStatDict().Get(Stat::Damage)},
                {"defense", monster->GetStatDict().Get(Stat::Defense)},
                {"special", monster->GetStatDict().Get(Stat::Special)},
            });
        else arr.push_back(nullptr);
    }
    return arr;
}

Monster *PlayerSession::GetMonsterByID(const std::string &id) const {
    for (Monster *m: Monsters) if (m && m->ID == id) return m;
    return nullptr;
}

bool PlayerSession::TryLevelMonster(const std::string &id, nlohmann::json data, std::string *err = nullptr) const {
    Monster *mon = GetMonsterByID(id);
    if (!mon) {
        if (err) *err = "Monster not found.";
        return false;
    }
    int totalSP = 0;
    for (const auto &[key, value]: data.items())
        totalSP += value.get<int>();
    if (totalSP > mon->GetStatDict().Get(Stat::SkillPoints)) {
        if (err) *err = "Not enough skill-points.";
        return false;
    }

    for (const auto &[key, value]: data.items()) {
        auto it = StringStatMap.find(key);
        if (it == StringStatMap.end()) {
            if (err) *err = "Stat [" + key + "] not found. SP's only partially applied.";
            return false;
        }
        for (int i = 0; i < value.get<int>(); i++)
            if (!mon->GetStatDict().TryLevel(it->second, err)) return false;
    }

    return true;
}

PlayerSession::~PlayerSession() {
    //TODO Delete monster when out of scope. ?????????
}
