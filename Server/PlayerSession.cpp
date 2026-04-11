#include "PlayerSession.h"
#include <ctime>
#include "Server/ServerHelpers.h"
using json = nlohmann::json;

PlayerSession::PlayerSession(const int playerID) : PlayerID(playerID), SessionID(RandomID()) {
    lastActivity = time(nullptr);
    // TODO
    Monsters[0] = new Human("George", 1);
    Monsters[1] = new Human("Peter", 2);
    Monsters[2] = new Orc("Murkk", 3);
    Monsters[3] = new Orc("Gurk", 4);
    Monsters[4] = nullptr;
}

bool PlayerSession::IsActive() const {
    return lastActivity + Config::Server::SessionTimeout > time(nullptr);
}

void PlayerSession::UpdateLastActivity() {
    lastActivity = time(nullptr);
}

json PlayerSession::GetMonsterJson(const int monsterID) const {
    json arr = json::array();
    for (Monster *monster: Monsters) {
        if (monsterID != -1 && (!monster || monster->ID != monsterID)) continue;
        if (monster)
            arr.push_back({
                {"id", monster->ID},
                {"name", monster->Name},
                {"type", MonsterDescriptions.at(monster->Type).TypeAsString},
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

bool PlayerSession::TryLevelMonster(const int id, nlohmann::json data, std::string *err = nullptr) const {
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

    for (const auto &[stat, lvlAmount]: data.items()) {
        auto it = StatStringMap.find(stat);
        if (it == StatStringMap.end()) {
            if (err) *err = "Stat [" + stat + "] not found. SP's only partially applied.";
            return false;
        }
        for (int i = 0; i < lvlAmount.get<int>(); i++)
            if (!mon->GetStatDict().TryLevel(StatStringMap.at(stat), err)) return false;
    }

    return true;
}

PlayerSession::~PlayerSession() {
    //TODO Delete monster when out of scope. ?????????
}

Monster *PlayerSession::GetMonsterByID(const int id) const {
    for (Monster *m: Monsters) if (m && m->ID == id) return m;
    return nullptr;
}
