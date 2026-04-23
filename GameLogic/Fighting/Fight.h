#ifndef ASCII_CLASH_FIGHT_H
#define ASCII_CLASH_FIGHT_H

#include "Database/JsonSavable.h"
#include "Log/NestedLogger.h"
#include "GameLogic/Team.h"
#include <array>

class Fight final : public JsonSavable {
    static constexpr int FIGHT_SIZE = 2;

public:
    struct TurnPair {
        Monster *attacker, *defender;
    };

    [[nodiscard]] Team *Winner() const { return winner; }

    explicit Fight(std::array<Team *, FIGHT_SIZE> teams);

    [[nodiscard]] nlohmann::json ToJson() const override;

    static Fight FromJson(const nlohmann::json &j, std::array<Team *, FIGHT_SIZE> teams);

    bool TryTakeTurn(const Team *initiator, int atkMonID, int defMonID, std::string *err = nullptr);

    const std::string ID;

    ~Fight() override;

private:
    [[nodiscard]] Team *ActiveTeam() const { return teams[turnIndex % FIGHT_SIZE]; }

    void ExecuteTurn(TurnPair mons);

    [[nodiscard]] TurnPair AutoPickMons() const;

    void UpdateWinner();

    void EndFight() const;

    NestedLogger log;
    Team *winner = nullptr;
    int turnIndex = 0;
    const std::array<Team *, FIGHT_SIZE> teams;
};

#endif
