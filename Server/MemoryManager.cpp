#include "MemoryManager.h"
#include "Database/TableDefs.h"
#include "Imports/httplib.h"

MemoryManager::MemoryManager(Database::SaveManager *saveManager) : saveManager(saveManager) {
}

PlayerSession *MemoryManager::TryGetPlayer(const std::string &sessionID, std::string *err) {
    const auto it = loadedSessions.find(sessionID);
    if (it == loadedSessions.end()) {
        SET_ERR("Session not found.");
        return nullptr;
    }
    if (it->second->CheckActive()) return it->second.get();
    SET_ERR("Session has expired.");
    return nullptr;
}

std::shared_ptr<Team> MemoryManager::TryGetTeam(const int id, std::string *err) {
    if (const auto it = loadedTeams.find(id); it != loadedTeams.end())
        if (auto t = it->second.lock()) return t;

    const auto teamJ = saveManager->LoadWhere(TABLE(Teams), {{COL(Teams, ID), std::to_string(id)}});
    if (teamJ.empty()) {
        SET_ERR("Team not found.");
        return nullptr;
    }
    auto team = std::make_shared<Team>(Team::FromJson(teamJ[0]));
    loadedTeams[id] = team;
    return team;
}

Fight *MemoryManager::TryGetFight(const int id, std::string *err) {
    if (const auto it = loadedFights.find(id); it != loadedFights.end()) return it->second.get();

    const auto fightJ = saveManager->LoadWhere(TABLE(Fights), {{COL(Fights, ID), std::to_string(id)}});
    if (fightJ.empty()) {
        SET_ERR("Fight not found.");
        return nullptr;
    }
    if (!fightJ[0][COL(Fights, is_ongoing)].get<bool>()) {
        SET_ERR("Fight is already over.");
        return nullptr;
    }

    std::vector<std::shared_ptr<Team> > participants;
    for (auto tj: fightJ[0][COL(Fights, participants)]) {
        auto t = TryGetTeam(tj[COL(Teams, ID)].get<int>(), err);
        participants.push_back(t);
        if (t) continue;
        SET_ERR("Not all Teams in the Fight could be loaded.");
        return nullptr;
    }
    loadedFights[id] = std::make_unique<Fight>(Fight::FromJson(fightJ[0], participants));
    return loadedFights[id].get();
}

int MemoryManager::TryCreateFight(const std::vector<int> &teamIDs, std::string *err) {
    std::vector<std::shared_ptr<Team> > teams;
    for (const int id: teamIDs) {
        auto t = TryGetTeam(id, err);
        teams.push_back(t);
        if (t && !t->IsInFight()) continue;
        SET_ERR("Not all required teams could be found and are available.");
        return -1;
    }
    auto f = std::make_unique<Fight>(Fight(teams));
    const auto id = f->ID;
    saveManager->SaveTo(TABLE(Fights), f->ToJson());
    loadedFights[f->ID] = std::move(f);
    return id;
}

std::string MemoryManager::TryRegisterPlayer(std::string username, const int password, std::string *err) {
    if (!saveManager->LoadWhere(TABLE(Players), {
                                    {COL(Players, username), username}
                                }).empty()) {
        SET_ERR("Username already exists.");
        return "";
    }

    const int id = saveManager->NextID(TABLE(Players));
    auto p = std::make_unique<PlayerSession>(PlayerSession(id, username, password));
    const std::string cookie = p->SessionID;
    saveManager->SaveTo(TABLE(Players), p->ToJson());
    loadedSessions[p->SessionID] = std::move(p);
    return cookie;
}

std::string MemoryManager::TryLoginPlayer(std::string username, const int password, std::string *err) {
    const std::vector<std::pair<std::string, std::string> > cons = {
        {COL(Players, username), username}, {COL(Players, password), std::to_string(password)}
    };
    auto playerJson = saveManager->LoadWhere(TABLE(Players), cons);
    if (playerJson.empty()) {
        SET_ERR("Player not found.");
        return "";
    }
    const int playerID = playerJson[0][COL(Players, ID)].get<int>();
    for (const auto &[cookie, session]: loadedSessions) {
        if (session->PlayerID != playerID) continue;
        if (session->CheckActive()) return cookie;
        SET_ERR("Session has expired.");
        return "";
    }


    auto teamIDs = playerJson[0][COL(Players, teams)];
    std::array<std::shared_ptr<Team>, Config::Player::TeamAmount> teams{};
    for (int i = 0; i < Config::Player::TeamAmount; i++) {
        if (teamIDs.size() <= i) {
            teams[i] = nullptr;
            continue;
        }
        teams[i] = TryGetTeam(teamIDs[i].get<int>(), err);
        if (!teams[i]) return "";
        const auto fights = saveManager->LoadWhere(TABLE(FightTeams), {
                                                       {COL(FightTeams, team_ID), std::to_string(teams[i]->ID)}
                                                   });
        for (const auto &f: fights) TryGetFight(f[COL(FightTeams, fight_ID)].get<int>(), err);
    }
    auto p = std::make_unique<PlayerSession>(PlayerSession::FromJson(playerJson[0], teams));
    const std::string cookie = p->SessionID;
    loadedSessions[p->SessionID] = std::move(p);
    return cookie;
}

void MemoryManager::TryLogoutPlayer(const std::string &cookie, std::string *err) {
    if (!loadedSessions.contains(cookie)) {
        SET_ERR("No session found.");
        return;
    }
    saveManager->SaveTo(TABLE(Players), loadedSessions[cookie]->ToJson());
    loadedSessions.erase(cookie);
    Cleanup();
}

void MemoryManager::Cleanup() {
    //Sessions
    for (auto it = loadedSessions.begin(); it != loadedSessions.end();) {
        if (it->second->CheckActive()) ++it;
        else {
            saveManager->SaveTo(TABLE(Players), it->second->ToJson());
            it = loadedSessions.erase(it);
        }
    }

    //Fights
    for (auto it = loadedFights.begin(); it != loadedFights.end();) {
        if (it->second->Winner()) {
            saveManager->SaveTo(TABLE(Fights), it->second->ToJson());
            it = loadedFights.erase(it);
            continue;
        }

        bool playerHoldsTeam = false;
        for (const auto &t: it->second->Teams()) {
            if (t.use_count() <= 1) continue;
            playerHoldsTeam = true;
            break;
        }
        if (!playerHoldsTeam) {
            saveManager->SaveTo(TABLE(Fights), it->second->ToJson());
            it = loadedFights.erase(it);
        } else ++it;
    }

    //Teams
    for (auto it = loadedTeams.begin(); it != loadedTeams.end();) {
        if (it->second.expired()) it = loadedTeams.erase(it);
        else ++it;
    }
}
