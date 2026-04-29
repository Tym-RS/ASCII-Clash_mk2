#ifndef ASCII_CLASH_PLAYERSESSION_H
#define ASCII_CLASH_PLAYERSESSION_H

#include "../../Config.h"
#include "GameLogic/Team.h"


class PlayerSession final : public JsonSavable {
public:
    explicit PlayerSession(int id, std::string username, int password);

    [[nodiscard]] nlohmann::json ToJson() const override;

    static PlayerSession FromJson(nlohmann::json j,
                                  std::array<std::shared_ptr<Team>, Config::Player::TeamAmount> &teams);

    [[nodiscard]] bool CheckActive();

    [[nodiscard]] int GetScore() const;

    std::shared_ptr<Team> TryGetCreateNewTeam(std::string name, ERR_PARAM);

    Team *TryGetTeam(int id, ERR_PARAM) const;

    bool TryDeleteTeam(int id, ERR_PARAM);

    std::array<std::shared_ptr<Team>, Config::Player::TeamAmount> *Teams() { return &teams; }
    const std::array<std::shared_ptr<Team>, Config::Player::TeamAmount> *Teams() const { return &teams; }

    const int PlayerID;
    const std::string SessionID;
    const std::string Username;

private:
    std::array<std::shared_ptr<Team>, Config::Player::TeamAmount> teams;
    const int password;
    long lastActivity = 0;
};

#endif
