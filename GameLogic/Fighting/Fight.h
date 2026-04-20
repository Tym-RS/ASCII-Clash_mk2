#ifndef ASCII_CLASH_FIGHT_H
#define ASCII_CLASH_FIGHT_H

#include "NestedLogger.h"
#include "Server/PlayerSession.h"
#include <array>

class Fight {
    static constexpr int fightSize = 2;

public:
    explicit Fight(std::array<PlayerSession *, fightSize> players);

    nlohmann::json AsJSON() const;

    ~Fight();

private:
    NestedLogger Log;
    PlayerSession *playerAtTurn = nullptr;
    const std::array<PlayerSession *, fightSize> players;
};

#endif
