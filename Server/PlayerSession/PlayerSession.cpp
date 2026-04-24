#include "PlayerSession.h"

#include <random>
#include <utility>
#include "Database/TableDefs.h"

#define JStr(at) COL(Players, at)

inline std::string RandomID() {
    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist;
    std::stringstream stream;
    for (int i = 0; i < 4; ++i) stream << std::hex << dist(rd);
    return stream.str();
}

PlayerSession::PlayerSession(const int id, std::string username, const int password) : PlayerID(id),
    Username(std::move(username)),
    password(password), SessionID(RandomID()) {
    lastActivity = time(nullptr);
}

nlohmann::json PlayerSession::ToJson() const {
    auto teamIDs = nlohmann::json::array();
    for (const auto &t: teams) if (t) teamIDs.emplace_back(t->ID);
    return {
        {JStr(ID), PlayerID},
        {JStr(teams), teamIDs},
        {JStr(score), Score},
        {JStr(password), password},
        {JStr(username), Username},
    };
}

PlayerSession PlayerSession::FromJson(nlohmann::json j,
                                      std::array<std::shared_ptr<Team>, Config::Player::TeamAmount> &teams) {
    const int id = j.at(JStr(ID)).get<int>();
    const std::string username = j.at(JStr(username)).get<std::string>();
    const int password = j.at(JStr(password)).get<int>();
    auto player = PlayerSession(id, username, password);
    player.Score = j.at(JStr(score)).get<int>();
    player.teams = std::move(teams);
    return player;
}

bool PlayerSession::CheckActive() {
    if (lastActivity + Config::Server::SessionTimeout < time(nullptr)) return false;
    lastActivity = time(nullptr);
    return true;
}

#undef JStr

//TODO
// #include <ctime>
// #include <utility>
// #include "Server/ServerHelpers.h"
// using json = nlohmann::json;
//
// PlayerSession::PlayerSession(const int playerID, std::string username) : PlayerID(playerID),
//                                                                          SessionID(RandomID()),
//                                                                          Username(std::move(username)), Monsters() {
//     lastActivity = time(nullptr);
// }
//
//
// bool PlayerSession::IsActive() const {
//     return lastActivity + Config::Server::SessionTimeout > time(nullptr);
// }
//
// void PlayerSession::UpdateLastActivity() {
//     lastActivity = time(nullptr);
// }
//
// json PlayerSession::GetMonsterJson(const int monsterID) const {
//     json arr = json::array();
//     for (Monster *monster: Monsters) {
//         if (monsterID != -1 && (!monster || monster->ID != monsterID)) continue;
//         if (monster)
//             arr.push_back({
//                 monster->ToJson()
//             });
//         else arr.push_back(nullptr);
//     }
//     return arr;
// }
//
// bool PlayerSession::TryLevelMonster(const int id, nlohmann::json data, std::string *err = nullptr) const {
//     Monster *mon = GetMonsterByID(id);
//     if (!mon) {
//         if (err) *err = "Monster not found.";
//         return false;
//     }
//     int totalSP = 0;
//     for (const auto &[key, value]: data.items())
//         totalSP += value.get<int>();
//     if (totalSP > mon->GetStatDict()->Get(Stat::SkillPoints)) {
//         if (err) *err = "Not enough skill-points.";
//         return false;
//     }
//
//     for (const auto &[stat, lvlAmount]: data.items()) {
//         if (!StatStringMap.contains(stat)) {
//             if (err) *err = "Stat [" + stat + "] not found. SP's only partially applied.";
//             return false;
//         }
//         for (int i = 0; i < lvlAmount.get<int>(); i++)
//             if (!mon->GetStatDict()->TryLevel(StatStringMap.at(stat), err)) return false;
//     }
//
//     return true;
// }
//
// PlayerSession::~PlayerSession() {
//     for (const Monster *monster: Monsters) delete monster;
// }
//
// Monster *PlayerSession::GetMonsterByID(const int id) const {
//     for (Monster *m: Monsters) if (m && m->ID == id) return m;
//     return nullptr;
// }
