#ifndef ASCII_CLASH_FIGHT_H
#define ASCII_CLASH_FIGHT_H

#include "Database/JsonSavable.h"
#include "Log/NestedLogger.h"
#include <array>

class Team;
class Monster;

class Fight final : public JsonSavable {
public:
    struct TurnPair {
        Monster *attacker, *defender;
    };

    [[nodiscard]] Team *Winner() const { return winner; }

    explicit Fight(const std::vector<std::shared_ptr<Team> > &teams);

    [[nodiscard]] nlohmann::json ToJson() const override;

    static Fight FromJson(const nlohmann::json &json, const std::vector<std::shared_ptr<Team> > &teams);

    bool TryTakeTurn(const Team *initiator, int atkMonID, int defMonID, std::string *err = nullptr);

    const std::vector<std::shared_ptr<Team> > &Teams() const { return teams; }
    const int ID;

private:
    const int fightSize;
    [[nodiscard]] Team *ActiveTeam() const { return teams[turnIndex % fightSize].get(); }

    void ExecuteTurn(TurnPair mons);

    [[nodiscard]] TurnPair GetRandomMonPair() const;

    void TryConclude();

    void EndFight() const;

    NestedLogger log;
    Team *winner = nullptr;
    int turnIndex = 0;
    const std::vector<std::shared_ptr<Team> > teams;
};

#endif
