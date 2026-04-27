#include "MemoryManager.h"

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

std::shared_ptr<Team> MemoryManager::GetNewAITeam(const int level) const {
    std::lock_guard lock(mtx);
    static int teamID = -1;
    auto t = std::make_shared<Team>(GetRandomTeamName(), teamID--);
    const auto mons = &t->Monsters();
    for (int i = 0; i < mons->size(); i++) {
        const auto type = static_cast<MonsterType>(rand() % static_cast<int>(MonsterType::COUNT));
        auto mon = CreateMonster(GetRandomMonName(), i, type);
        const int offsetLvl = level + (rand() % 3 - 1);
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
    for (auto tj: fightJ[0][COL(Fights, participants)]) {
        const int teamID = tj[COL(Teams, ID)].get<int>();
        std::shared_ptr<Team> team;
        if (teamID >= 0) team = TryGetTeam(tj[COL(Teams, ID)].get<int>(), err);
        else {
            // AI-Team
            team = std::make_shared<Team>(Team::FromJson(tj));
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
    for (const int id: teamIDs) {
        auto t = TryGetTeam(id, err);
        teams.push_back(t);
        if (t && !t->IsInFight()) continue;
        SET_ERR("Not all required teams could be found and are available.");
        return -1;
    }
    auto f = std::make_unique<Fight>(Fight(teams));
    const auto id = f->ID;
    if (!saveManager->TrySaveTo(TABLE(Fights), f->ToJson(), err)) return -1;
    loadedFights[f->ID] = std::move(f);
    return id;
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

int MemoryManager::TryGetNewTeamID(const std::string &cookie, const std::string &name, std::string *err) {
    std::lock_guard lock(mtx);
    const auto player = TryGetPlayer(cookie, err);
    if (!player) return -1;
    const auto team = player->TryGetCreateNewTeam(name, err);
    if (!team) return -1;
    if (!saveManager->TrySaveTo(TABLE(Teams), team->ToJson(), err)) return -1;
    if (!saveManager->TrySaveTo(TABLE(Players), player->ToJson(), err)) return -1;
    if (!TryGetTeam(team->ID, err))return -1;
    return team->ID;
}

bool MemoryManager::TryDeleteTeam(const std::string &cookie, const int id, std::string *err) {
    std::lock_guard lock(mtx);
    const auto player = TryGetPlayer(cookie, err);
    if (!player) return false;
    if (!TryGetTeam(id, err)) return false;
    if (!player->TryDeleteTeam(id, err)) return false;
    saveManager->TrySaveTo(TABLE(Players), player->ToJson(), err);
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
    //Sessions
    for (auto it = loadedSessions.begin(); it != loadedSessions.end();) {
        if (it->second->CheckActive()) ++it;
        else {
            saveManager->TrySaveTo(TABLE(Players), it->second->ToJson());
            it = loadedSessions.erase(it);
        }
    }

    //Fights
    for (auto it = loadedFights.begin(); it != loadedFights.end();) {
        if (it->second->Winner()) {
            saveManager->TrySaveTo(TABLE(Fights), it->second->ToJson());
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
            saveManager->TrySaveTo(TABLE(Fights), it->second->ToJson());
            it = loadedFights.erase(it);
        } else ++it;
    }

    //Teams
    for (auto it = loadedTeams.begin(); it != loadedTeams.end();) {
        if (it->second.expired()) it = loadedTeams.erase(it);
        else ++it;
    }
}

void MemoryManager::SaveAll() {
    std::lock_guard lock(mtx);
    for (const auto &p: loadedSessions | std::views::values)
        saveManager->TrySaveTo(TABLE(Players), p->ToJson());
    for (const auto &f: loadedFights | std::views::values)
        saveManager->TrySaveTo(TABLE(Fights), f->ToJson());
    for (const auto &tPtr: loadedTeams | std::views::values)
        if (const auto t = tPtr.lock())
            saveManager->TrySaveTo(TABLE(Teams), t->ToJson());
}
