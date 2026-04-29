#include "MemoryManager.h"
#include <iostream>
#include "AI_Names.h"
#include "Database/TableDefs.h"
#include "GameLogic/Monsters/CreateMonster.h"

MemoryManager::MemoryManager(Database::SaveManager *saveManager) : saveManager(saveManager) {
}

PlayerSession *MemoryManager::TryGetPlayer(const std::string &sessionID, std::string *err) {
    std::lock_guard lock(mtx);
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
    std::lock_guard lock(mtx);
    if (const auto it = loadedTeams.find(id); it != loadedTeams.end())
        if (auto t = it->second.lock()) return t;

    const auto teamJ = saveManager->LoadWhere(TABLE(Teams), {{COL(Teams, ID), std::to_string(id)}}, "", -1, err);
    if (teamJ.empty()) {
        SET_ERR("Team not found.");
        return nullptr;
    }
    auto team = std::make_shared<Team>(Team::FromJson(teamJ[0]));
    loadedTeams[id] = team;
    return team;
}

bool MemoryManager::TryOpenFightRequest(const std::shared_ptr<Team> &team, std::string *err) {
    if (team->IsInFight()) {
        SET_ERR("Team already in a fight.");
        return false;
    }
    if (team->GetLvl() <= 0) {
        SET_ERR("Empty teams canNOT fight.");
        return false;
    }
    fightRequestingTeams[team->ID] = team;
    return true;
}

bool MemoryManager::RetractFightRequest(const std::shared_ptr<Team> &team, std::string *err) {
    if (fightRequestingTeams.contains(team->ID))
        fightRequestingTeams.erase(team->ID);
    return true;
}

nlohmann::json MemoryManager::GetFightRequests() {
    std::vector<int> toErase;
    for (auto &[id, team]: fightRequestingTeams)
        if (team.expired()) toErase.push_back(id);
    for (const int id: toErase) fightRequestingTeams.erase(id);

    nlohmann::json j = nlohmann::json::array();
    for (auto &team: fightRequestingTeams | std::views::values) {
        const auto t = team.lock();
        j.push_back({
            {COL(Teams, name), t->Name},
            {COL(Teams, ID), t->ID},
            {"Level", t->GetLvl()},
        });
    }
    return j;
}

void MemoryManager::CleanFightRequests() {
    std::vector<int> toErase;
    for (auto &[id, team]: fightRequestingTeams)
        if (team.expired() || team.lock()->IsInFight() || team.lock()->GetLvl() <= 0)
            toErase.push_back(id);
    for (const int id: toErase) fightRequestingTeams.erase(id);
}

std::shared_ptr<Team> MemoryManager::GetNewAITeam(const int level) const {
    std::lock_guard lock(mtx);
    static int teamID = -1;
    auto t = std::make_shared<Team>(GetRandomTeamName(), teamID--);
    const auto mons = &t->Monsters();
    for (int i = 0; i < mons->size(); i++) {
        const auto type = static_cast<MonsterType>(rand() % static_cast<int>(MonsterType::COUNT));
        auto mon = CreateMonster(GetRandomMonName(), i, type);
        const int offsetLvl = level / mons->size() + (rand() % 3 - 1);
        mon->GetStatDict()->ReceiveEXP(offsetLvl * (offsetLvl + 1));
        while (mon->GetStatDict()->Get(Stat::SkillPoints) > 0)
            mon->GetStatDict()->TryLevel(static_cast<Stat>(rand() % static_cast<int>(Stat::COUNT)));
        (*mons)[i] = std::move(mon);
    }
    t->AutoFight = true;
    return t;
}

Fight *MemoryManager::TryGetFight(const int id, std::string *err) {
    std::lock_guard lock(mtx);
    if (const auto it = loadedFights.find(id); it != loadedFights.end()) return it->second.get();

    const auto fightJ = saveManager->LoadWhere(TABLE(Fights), {{COL(Fights, ID), std::to_string(id)}});
    if (fightJ.empty()) {
        SET_ERR("Fight not found.");
        return nullptr;
    }
    if (!fightJ[0][COL(Fights, is_ongoing)].get<bool>()) {
        SET_ERR("Fight is already over.");
        Cleanup();
        return nullptr;
    }

    std::vector<std::shared_ptr<Team> > participants;
    for (auto teamJ: fightJ[0][COL(Fights, participants)]) {
        const int teamID = teamJ[COL(Teams, ID)].get<int>();
        std::shared_ptr<Team> team = nullptr;
        team = TryGetTeam(teamJ[COL(Teams, ID)].get<int>(), err);
        if (!team) {
            // AI-Team OR use saved teamJ as fallback.
            team = std::make_shared<Team>(Team::FromJson(teamJ));
            loadedTeams[teamID] = team;
        }
        participants.push_back(team);
        if (team) continue;
        SET_ERR("Not all Teams in the Fight could be loaded.");
        return nullptr;
    }
    loadedFights[id] = std::make_unique<Fight>(Fight::FromJson(fightJ[0], participants));
    return loadedFights[id].get();
}

int MemoryManager::TryCreateFight(const std::vector<int> &teamIDs, std::string *err) {
    std::lock_guard lock(mtx);
    std::vector<std::shared_ptr<Team> > teams;
    int bestTeamLevel = 0;

    for (const int id: teamIDs) {
        if (id == -1) continue;
        auto t = TryGetTeam(id, err);
        if (!t || t->IsInFight()) {
            SET_ERR("Not all required teams could be found and are available.");
            return -1;
        }
        if (!fightRequestingTeams.contains(id)) {
            SET_ERR("Team " + t->Name + " is not willing to fight.");
            return -1;
        }
        if (const int teamLvl = t->GetLvl(); teamLvl > bestTeamLevel) bestTeamLevel = teamLvl;
        teams.push_back(t);
    }

    for (const int id: teamIDs)
        if (id == -1) teams.push_back(GetNewAITeam(bestTeamLevel));

    auto f = std::make_unique<Fight>(Fight(teams));
    const auto id = f->ID;
    Save(f.get());
    loadedFights[f->ID] = std::move(f);
    return id;
}

nlohmann::json MemoryManager::GetLeaderboard(const int limit) const {
    const auto players = saveManager->LoadWhere(TABLE(Players), {}, COL(Players, score), limit);
    nlohmann::json j = nlohmann::json::array();
    for (auto p: players)
        j.push_back({
                {COL(Players, username), p[COL(Players, username)]},
                {COL(Players, score), p[COL(Players, score)]},
            }
        );
    return j;
}

bool MemoryManager::TryRegisterPlayer(std::string username, const int password, std::string *err) const {
    std::lock_guard lock(mtx);
    if (!saveManager->LoadWhere(TABLE(Players), {
                                    {COL(Players, username), username}
                                }).empty()) {
        SET_ERR("Username already exists.");
        return false;
    }
    const int id = saveManager->NextID(TABLE(Players));
    saveManager->TrySaveTo(TABLE(Players), PlayerSession(id, username, password).ToJson());
    return true;
}


std::string MemoryManager::GetNewSessionID(std::string username, const int password, std::string *err) {
    std::lock_guard lock(mtx);
    const std::vector<std::pair<std::string, std::string> > conditions = {
        {COL(Players, username), username}, {COL(Players, password), std::to_string(password)}
    };
    auto playerJson = saveManager->LoadWhere(TABLE(Players), conditions);
    if (playerJson.empty()) {
        SET_ERR("Player not found.");
        return "";
    }

    const int playerID = playerJson[0][COL(Players, ID)].get<int>();
    for (const auto &[cookie, session]: loadedSessions) {
        if (session->PlayerID != playerID) continue;
        if (session->CheckActive()) return cookie;
        Cleanup();
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
        if (teams[i]->IsInFight())TryGetFight(teams[i]->CurrentFightID(), err);
    }
    auto p = std::make_unique<PlayerSession>(PlayerSession::FromJson(playerJson[0], teams));
    const std::string cookie = p->SessionID;
    loadedSessions[p->SessionID] = std::move(p);
    return cookie;
}

void MemoryManager::TryLogoutPlayer(const std::string &cookie, std::string *err) {
    std::lock_guard lock(mtx);
    if (!loadedSessions.contains(cookie)) {
        SET_ERR("No session found.");
        return;
    }
    saveManager->TrySaveTo(TABLE(Players), loadedSessions[cookie]->ToJson(), err);
    loadedSessions.erase(cookie);
    Cleanup();
}

void MemoryManager::Cleanup() {
    std::lock_guard lock(mtx);
    Save();

    // Players/Sessions
    for (auto it = loadedSessions.begin(); it != loadedSessions.end();) {
        if (it->second->CheckActive()) ++it;
        else it = loadedSessions.erase(it);
    }

    //Fights
    for (auto it = loadedFights.begin(); it != loadedFights.end();) {
        if (it->second->Winner()) {
            it = loadedFights.erase(it);
            continue;
        }
        bool playerHoldsTeam = false;
        for (const auto &t: it->second->Teams()) {
            if (t.use_count() <= 1) continue;
            playerHoldsTeam = true;
            break;
        }
        if (!playerHoldsTeam) it = loadedFights.erase(it);
        else ++it;
    }

    // Teams
    for (auto it = loadedTeams.begin(); it != loadedTeams.end();) {
        if (it->second.expired()) it = loadedTeams.erase(it);
        else ++it;
    }
}

void MemoryManager::Save() {
    std::lock_guard lock(mtx);
    for (const auto &p: loadedSessions | std::views::values)
        saveManager->TrySaveTo(TABLE(Players), p->ToJson());
    for (const auto &f: loadedFights | std::views::values)
        saveManager->TrySaveTo(TABLE(Fights), f->ToJson());
    for (const auto &tPtr: loadedTeams | std::views::values)
        if (const auto t = tPtr.lock())
            saveManager->TrySaveTo(TABLE(Teams), t->ToJson());
}

void MemoryManager::Save(const PlayerSession *player) {
    std::lock_guard lock(mtx);
    saveManager->TrySaveTo(TABLE(Players), player->ToJson());
    for (const auto &t: *player->Teams()) {
        if (!t)continue;
        saveManager->TrySaveTo(TABLE(Teams), t->ToJson());
        loadedTeams[t->ID] = t;
    }
}

void MemoryManager::Save(const Fight *fight) {
    std::lock_guard lock(mtx);
    saveManager->TrySaveTo(TABLE(Fights), fight->ToJson());
    for (auto &t: fight->Teams()) {
        saveManager->TrySaveTo(TABLE(Teams), t->ToJson());
        loadedTeams[t->ID] = t;
    }
}
