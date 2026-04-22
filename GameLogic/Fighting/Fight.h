#ifndef ASCII_CLASH_FIGHT_H
#define ASCII_CLASH_FIGHT_H

#include "Log/NestedLogger.h"
#include <array>

#include "GameLogic/Team.h"

class Fight final : public JsonSavable<Fight> {
    static constexpr int fightSize = 2;

public:
    [[nodiscard]] Team *GetWinner() const { return winner; }

    explicit Fight(std::array<Team *, fightSize> teams);


    [[nodiscard]] nlohmann::json ToJson() override;

    static Fight *FromJson(const nlohmann::json &j);

    bool TryTakeTurn(const Team *initiator, int atkMonID, int defMonID,
                     std::string *err = nullptr);

    const std::string ID;

    ~Fight() override;

private:
    struct MonTuple {
        Monster *attacker, *defender;
    };

    [[nodiscard]] Team *ActiveTeam() const { return teams[turnIndex % fightSize]; }

    void ExecuteTurn(MonTuple mons);

    [[nodiscard]] MonTuple AutoPickMons() const;

    void UpdateWinner();

    NestedLogger log;
    Team *winner = nullptr;
    int turnIndex = 0;
    const std::array<Team *, fightSize> teams;
};

#endif
