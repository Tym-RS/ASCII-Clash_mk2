#include "Fight.h"
#include <string>
#include <cstdlib>

Fight::Fight(const std::array<PlayerSession *, fightSize> players)
    : players(players) {
    std::string startMessage = "Fight between ";
    for (size_t i = 0; i < fightSize; ++i) {
        const auto *p = players[i];
        startMessage += p->Username;
        if (i + 1 < players.size()) startMessage += " and ";
        for (auto *m: p->Monsters) if (m) m->LogPtr = &Log;
    }
    startMessage += ".";
    Log.Append(startMessage, LType::Minor);

    playerAtTurn = players[std::rand() % fightSize];
    Log.Append(playerAtTurn->Username + " starts!", LType::Major);
}



Fight::~Fight() {
    for (const auto p: players)
        for (const auto m: p->Monsters)
            if (m) m->LogPtr = nullptr;
}
