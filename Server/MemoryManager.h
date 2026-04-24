#ifndef ASCII_CLASH_MEMORYMANAGER_H
#define ASCII_CLASH_MEMORYMANAGER_H

#include "Database/SaveManager.h"
#include "GameLogic/Team.h"
#include "PlayerSession/PlayerSession.h"
#include "ErrHelper.h"

class MemoryManager {
public:
    explicit MemoryManager(Database::SaveManager *saveManager);

    PlayerSession *TryGetPlayer(const std::string &sessionID, ERR_PARAM);

    std::shared_ptr<Team> TryGetTeam(int id, ERR_PARAM);

    Fight *TryGetFight(int id, ERR_PARAM);

    int TryCreateFight(const std::vector<int> &teamIDs, ERR_PARAM);

    std::string TryRegisterPlayer(std::string username, int password, ERR_PARAM);

    std::string TryLoginPlayer(std::string username, int password, ERR_PARAM);

    void TryLogoutPlayer(const std::string &cookie, ERR_PARAM);

    void Cleanup();

private:
    Database::SaveManager *saveManager;

    std::map<std::string, std::unique_ptr<PlayerSession> > loadedSessions{};
    std::map<int, std::weak_ptr<Team> > loadedTeams{};;
    std::map<int, std::unique_ptr<Fight> > loadedFights{};
};


#endif
