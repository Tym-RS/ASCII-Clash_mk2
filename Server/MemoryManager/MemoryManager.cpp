#include "MemoryManager.h"
#include <iostream>
#include "_AINames.h"
#include "Database/TableDefs.h"
#include "GameLogic/Monsters/CreateMonster.h"

MemoryManager::MemoryManager(std::unique_ptr<Database::SaveManager> saveManager) : saveManager(std::move(saveManager)) {
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
    if (team->GetAverageLvl() <= 0) {
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
            {"Level", t->GetAverageLvl()},
        });
    }
    return j;
}

void MemoryManager::CleanFightRequests() {
    std::vector<int> toErase;
    for (auto &[id, team]: fightRequestingTeams)
        if (team.expired()) toErase.push_back(id);
        else if (const auto t = team.lock(); t->IsInFight() || t->GetAverageLvl() <= 0)
            toErase.push_back(id);
    for (const int id: toErase) fightRequestingTeams.erase(id);
}

std::shared_ptr<Team> MemoryManager::GetNewAITeam(const int level) const {
    std::lock_guard lock(mtx);
    static int teamID = -1;
    auto team = std::make_shared<Team>(GetRandomTeamName(), teamID--);
    const auto mons = &team->Monsters();
    for (int i = 0; i < mons->size(); i++) {
        const auto type = static_cast<MonsterType>(rand() % static_cast<int>(MonsterType::COUNT));
        auto mon = CreateMonster(GetRandomMonName(), i, type);
        const int offsetLvl = std::max(level + (rand() % 6 - 4), 0);
        mon->GetStatDict()->ReceiveEXP(offsetLvl * (offsetLvl + 1));
        while (mon->GetStatDict()->Get(Stat::SkillPoints) > 0)
            mon->GetStatDict()->TryLevel(static_cast<Stat>(rand() % static_cast<int>(Stat::COUNT)));
        mon->Reset();
        (*mons)[i] = std::move(mon);
    }
    team->AutoFight = true;
    return team;
}

Fight *MemoryManager::TryGetFight(const int id, std::string *err) {
    std::lock_guard lock(mtx);
    if (const auto it = loadedFights.find(id); it != loadedFights.end()) return it->second.get();

    const auto fightJ = TryGetFightJson(id, err);
    if (fightJ.empty()) {
        SET_ERR(*err);
        return nullptr;
    }

    if (!fightJ[COL(Fights, is_ongoing)].get<bool>()) {
        SET_ERR("Fight is already over.");
        Cleanup();
        return nullptr;
    }
    std::vector<std::shared_ptr<Team> > participants;
    for (auto teamJ: fightJ[COL(Fights, participants)]) {
        const int teamID = teamJ[COL(Teams, ID)].get<int>();
        std::shared_ptr<Team> team = nullptr;
        team = TryGetTeam(teamJ[COL(Teams, ID)].get<int>(), err);
        if (!team) {
            // AI-Team OR use saved teamJ as fallback.
            team = std::make_shared<Team>(Team::FromJson(teamJ));
            loadedTeams[teamID] = team;
        }
        participants.push_back(team);
    }
    loadedFights[id] = std::make_unique<Fight>(Fight::FromJson(fightJ, participants));
    return loadedFights[id].get();
}

nlohmann::json MemoryManager::TryGetFightJson(const int id, std::string *err) {
    Save();
    const auto fightJ = saveManager->LoadWhere(TABLE(Fights), {{COL(Fights, ID), std::to_string(id)}});
    if (fightJ.empty()) {
        SET_ERR("Fight not found.");
        return nullptr;
    }
    return fightJ[0];
}

int MemoryManager::TryCreateFight(const std::vector<int> &teamIDs, std::string *err) {
    std::lock_guard lock(mtx);
    std::vector<std::shared_ptr<Team> > teams;
    int bestTeamLevel = 0;

    for (const int id: teamIDs) {
        if (id == -1) continue;
        auto t = TryGetTeam(id, err);
        if (!t || t->IsInFight() || t->GetAverageLvl() <= 0) {
            SET_ERR("Not all required teams could be found and are available.");
            return -1;
        }
        if (const int teamLvl = t->GetAverageLvl(); teamLvl > bestTeamLevel) bestTeamLevel = teamLvl;
        teams.push_back(t);
    }
    for (const int id: teamIDs)
        if (id == -1) teams.push_back(GetNewAITeam(bestTeamLevel));

    auto fight = std::make_unique<Fight>(teams);
    const auto id = fight->ID;
    Save(fight.get());
    loadedFights[fight->ID] = std::move(fight);
    return id;
}

nlohmann::json MemoryManager::GetLeaderboard(const int limit) const {
    const auto players = saveManager->LoadWhere(TABLE(Players), {}, COL(Players, score), limit);
    nlohmann::json data = nlohmann::json::array();
    for (auto &p: players)
        data.push_back({
            {COL(Players, username), p[COL(Players, username)]},
            {COL(Players, score), p[COL(Players, score)]},
        });
    std::ranges::reverse(data);
    return data;
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
    if (!saveManager->TrySaveTo(TABLE(Players), PlayerSession(id, username, password).ToJson(), err))
        std::cerr << *err << std::endl;

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

    // Check for existing active session — safe iteration, no mutation
    for (const auto &[cookie, session]: loadedSessions) {
        if (session->PlayerID != playerID) continue;
        if (session->CheckActive()) return cookie;
        // Session found but expired — clean up after iteration
        break;
    }
    Cleanup();

    auto teamIDs = playerJson[0][COL(Players, teams)];
    std::array<std::shared_ptr<Team>, Config::Player::TeamAmount> teams{};
    for (int i = 0; i < Config::Player::TeamAmount; i++) {
        if (i >= teamIDs.size()) {
            teams[i] = nullptr;
            continue;
        }
        teams[i] = TryGetTeam(teamIDs[i].get<int>(), err);
        if (!teams[i])
            std::cerr << "CORRUPT PLAYER DATA  [" << std::to_string(playerID) << "]: TEAM NOT FOUND" << std::endl;
        if (teams[i] && teams[i]->IsInFight()) TryGetFight(teams[i]->CurrentFightID(), err);
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
    if (!saveManager->TrySaveTo(TABLE(Players), loadedSessions[cookie]->ToJson(), err))
        std::cerr << *err << std::endl;
    loadedSessions.erase(cookie);
    Cleanup();
}

void MemoryManager::Cleanup() {
    Save();

    // Players/Sessions
    for (auto it = loadedSessions.begin(); it != loadedSessions.end();) {
        if (it->second->CheckActive()) ++it;
        else it = loadedSessions.erase(it);
    }

    // Fights
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
    std::string err;
    for (const auto &p: loadedSessions | std::views::values)
        if (!saveManager->TrySaveTo(TABLE(Players), p->ToJson(), &err))
            std::cerr << err << std::endl;

    for (const auto &f: loadedFights | std::views::values)
        if (!saveManager->TrySaveTo(TABLE(Fights), f->ToJson(), &err))
            std::cerr << err << std::endl;

    for (const auto &tPtr: loadedTeams | std::views::values)
        if (const auto t = tPtr.lock())
            if (!saveManager->TrySaveTo(TABLE(Teams), t->ToJson(), &err))
                std::cerr << err << std::endl;
}


void MemoryManager::Save(const PlayerSession *player) {
    std::lock_guard lock(mtx);
    std::string err;
    if (!saveManager->TrySaveTo(TABLE(Players), player->ToJson(), &err))
        std::cerr << err << std::endl;

    for (const auto &t: *player->Teams()) {
        if (!t) continue;
        if (!saveManager->TrySaveTo(TABLE(Teams), t->ToJson(), &err))
            std::cerr << err << std::endl;
        loadedTeams[t->ID] = t;
    }
}

void MemoryManager::Save(const Fight *fight) {
    std::lock_guard lock(mtx);
    std::string err;
    if (!saveManager->TrySaveTo(TABLE(Fights), fight->ToJson(), &err))
        std::cerr << err << std::endl;
    for (auto &t: fight->Teams()) {
        if (!saveManager->TrySaveTo(TABLE(Teams), t->ToJson(), &err))
            std::cerr << err << std::endl;
        loadedTeams[t->ID] = t;
    }
}

#ifndef NDEBUG
void MemoryManager::DebugDump() {
    std::lock_guard lock(mtx);

    const std::string divider(60, '=');
    const std::string subDivider(60, '-');

    std::cout << divider << "\n";
    std::cout << "  MEMORY MANAGER DEBUG DUMP\n";
    std::cout << divider << "\n\n";

    // ── Loaded Sessions ──────────────────────────────────────────
    std::cout << "[ LOADED SESSIONS ]  (" << loadedSessions.size() << ")\n";
    std::cout << subDivider << "\n";
    if (loadedSessions.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto &[cookie, session]: loadedSessions) {
            std::cout << "  Session ID : " << cookie << "\n";
            std::cout << "  Player ID  : " << session->PlayerID << "\n";
            std::cout << "  Active     : " << (session->CheckActive() ? "yes" : "no (expired)") << "\n";
            std::cout << "  Data       : " << session->ToJson().dump(4) << "\n";
            std::cout << subDivider << "\n";
        }
    }
    std::cout << "\n";

    // ── Loaded Teams ─────────────────────────────────────────────
    std::cout << "[ LOADED TEAMS ]  (" << loadedTeams.size() << ")\n";
    std::cout << subDivider << "\n";
    if (loadedTeams.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto &[id, weakTeam]: loadedTeams) {
            if (const auto team = weakTeam.lock()) {
                std::cout << "  Team ID    : " << id << "\n";
                std::cout << "  Name       : " << team->Name << "\n";
                std::cout << "  Level      : " << team->GetAverageLvl() << "\n";
                std::cout << "  In Fight   : " << (team->IsInFight()
                                                       ? "yes (Fight #" + std::to_string(team->CurrentFightID()) + ")"
                                                       : "no") << "\n";
                std::cout << "  AI Team    : " << (team->AutoFight ? "yes" : "no") << "\n";
                std::cout << "  Data       : " << team->ToJson().dump(4) << "\n";
            } else {
                std::cout << "  Team ID    : " << id << "  [EXPIRED weak_ptr]\n";
            }
            std::cout << subDivider << "\n";
        }
    }
    std::cout << "\n";

    // ── Fight Requesting Teams ────────────────────────────────────
    std::cout << "[ FIGHT REQUESTING TEAMS ]  (" << fightRequestingTeams.size() << ")\n";
    std::cout << subDivider << "\n";
    if (fightRequestingTeams.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto &[id, weakTeam]: fightRequestingTeams) {
            if (const auto team = weakTeam.lock()) {
                std::cout << "  Team ID    : " << id << "\n";
                std::cout << "  Name       : " << team->Name << "\n";
                std::cout << "  Level      : " << team->GetAverageLvl() << "\n";
            } else {
                std::cout << "  Team ID    : " << id << "  [EXPIRED weak_ptr]\n";
            }
            std::cout << subDivider << "\n";
        }
    }
    std::cout << "\n";

    // ── Loaded Fights ─────────────────────────────────────────────
    std::cout << "[ LOADED FIGHTS ]  (" << loadedFights.size() << ")\n";
    std::cout << subDivider << "\n";
    if (loadedFights.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto &[id, fight]: loadedFights) {
            std::cout << "  Fight ID   : " << id << "\n";
            std::cout << "  Ongoing    : " << (fight->Winner() ? "no (finished)" : "yes") << "\n";
            std::cout << "  Teams      : ";
            for (const auto &t: fight->Teams())
                std::cout << t->Name << " (ID=" << t->ID << ")  ";
            std::cout << "\n";
            auto j = fight->ToJson();
            j[COL(Fights, log)] = {"..."};
            std::cout << "  Data       : " << j.dump(4) << "\n";
            std::cout << subDivider << "\n";
        }
    }
    std::cout << "\n";

    std::cout << divider << "\n";
    std::cout << "  END OF DUMP\n";
    std::cout << divider << "\n" << std::flush;
}
#endif
