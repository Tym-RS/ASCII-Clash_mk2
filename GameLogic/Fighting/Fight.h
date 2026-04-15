#ifndef ASCII_CLASH_FIGHT_H
#define ASCII_CLASH_FIGHT_H
#include "FightLogger.h"
#include "Server/PlayerSession.h"
#include <array>

class Fight {
    static constexpr int fightSize = 2;

public:
    explicit Fight(const std::array<PlayerSession *, fightSize> players) : players(players) {
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
        Log.AppendLog(playerAtTurn->Username + " starts!", LType::Major);
    }


    FightLogger Log = FightLogger();

    ~Fight() {
        for (const auto p: players)
            for (const auto m: p->Monsters)
                if (m) m->LogPtr = nullptr;
    }

private:
    PlayerSession *playerAtTurn = nullptr;
    const std::array<PlayerSession *, fightSize> players;
};


#endif
