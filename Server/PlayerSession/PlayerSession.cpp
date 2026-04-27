#include "Database/TableDefs.h"
#include "PlayerSession.h"
#include <utility>
#include <random>

#define JStr(at) COL(Players, at)

inline std::string RandomID() {
    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist;
    std::stringstream stream;
    for (int i = 0; i < 4; ++i) stream << std::hex << dist(rd);
    return stream.str();
}

PlayerSession::PlayerSession(const int id, std::string username, const int password) : PlayerID(id),
    SessionID(RandomID()),
    Username(std::move(username)), password(password) {
    lastActivity = time(nullptr);
}

nlohmann::json PlayerSession::ToJson() const {
    auto teamIDs = nlohmann::json::array();
    for (const auto &t: teams) if (t) teamIDs.emplace_back(t->ID);
    return {
        {JStr(ID), PlayerID},
        {JStr(teams), teamIDs},
        {JStr(score), GetScore()},
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
    player.teams = std::move(teams);
    return player;
}

bool PlayerSession::CheckActive() {
    if (lastActivity + Config::Server::SessionTimeout < time(nullptr)) return false;
    lastActivity = time(nullptr);
    return true;
}

Monster *PlayerSession::TryGetMonster(const int id) const {
    for (const auto &t: teams)
        for (const auto &m: t->Monsters())
            if (m && m->ID == id) return m.get();
    return nullptr;
}

int PlayerSession::GetScore() const {
    int score = 0;
    for (const auto t: teams)
        if (t)
            for (const auto &m: t->Monsters())
                if (m) score += m->GetStatDict()->Get(Stat::Level);
    return score;
}

std::shared_ptr<Team> PlayerSession::TryGetCreateNewTeam(std::string name, std::string *err) {
    int nextFree = 0;
    for (; nextFree < teams.size(); nextFree++)
        if (!teams[nextFree]) break;

    if (nextFree == teams.size()) {
        SET_ERR("No free team slot.");
        return nullptr;
    }
    teams[nextFree] = std::make_shared<Team>(name, PlayerID * Config::Player::TeamAmount + nextFree);
    return teams[nextFree];
}

bool PlayerSession::TryDeleteTeam(const int id, std::string *err) {
    for (int i = 0; i < teams.size(); i++) {
        if (!teams[i] || teams[i]->ID != id) continue;
        if (teams[i]->IsInFight()) {
            SET_ERR("Team is currently in a fight.");
            return false;
        }
        teams[i] = nullptr;
        return true;
    }
    SET_ERR("Team not found.");
    return false;
}

#undef JStr
