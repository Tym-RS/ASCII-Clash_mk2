#ifndef ASCII_CLASH_FIGHT_H
#define ASCII_CLASH_FIGHT_H

#include "NestedLogger.h"
#include "Server/PlayerSession.h"
#include <array>

class Fight {
    static constexpr int fightSize = 2;

public:
    [[nodiscard]] PlayerSession *GetWinner() const { return winner; }

    explicit Fight(std::array<PlayerSession *, fightSize> players);


    [[nodiscard]] nlohmann::json AsJson() const;

    bool TryTakeTurn(const PlayerSession *initiator, int atkMonID, int defMonID,
                     std::string *err = nullptr);

    const std::string ID;

    ~Fight();

private:
    struct MonTuple {
        Monster *attacker, *defender;
    };

    [[nodiscard]] PlayerSession *ActivePlayer() const { return players[turnIndex % fightSize]; }

    void ExecuteTurn(MonTuple mons);

    MonTuple AutoPickMons() const;

    void UpdateWinner();

    NestedLogger log;
    PlayerSession *winner = nullptr;
    int turnIndex = 0;
    const std::array<PlayerSession *, fightSize> players;
};

#endif
