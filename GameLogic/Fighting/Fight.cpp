#include "Fight.h"
#include <string>
#include <cstdlib>

Fight::Fight(const std::array<PlayerSession *, fightSize> players)
    : ID([&] {
        std::size_t h = 0;
        for (const auto p: players)
            h ^= (std::hash<std::string>{}(p->SessionID) << 1);
        return std::to_string(h);
    }()), players(players) {
    std::string startMessage = "Fight between ";
    for (size_t i = 0; i < fightSize; ++i) {
        const auto *p = players[i];
        startMessage += p->Username;
        if (i + 1 < players.size()) startMessage += " and ";
        for (auto *m: p->Monsters) if (m) m->LogPtr = &log;
    }
    startMessage += ".";
    log.Append(startMessage, LType::Minor);

    turnIndex = std::rand() % fightSize;
    log.Append(ActivePlayer()->Username + " starts!", LType::Major);
}

nlohmann::json Fight::AsJson() const {
    nlohmann::json pj = nlohmann::json::array();
    for (const auto p: players) {
        int aliveCount = 0, totalCurrentHP = 0;
        for (const auto m: p->Monsters) {
            if (!m || !m->CheckIsAlive()) continue;
            aliveCount++;
            totalCurrentHP += m->GetCurrentHealth();
        }
        pj.push_back({
            {"username", p->Username},
            {"autoFight", p->AutoFight},
            {"monsters", p->GetMonsterJson()},
            {"alive_count", aliveCount},
            {"total_hp", totalCurrentHP}
        });
    }

    return {
        {"id", ID},
        {"is_ongoing", !winner},
        {"winner", winner ? winner->Username : nullptr},
        {
            "active_player", {
                {"username", ActivePlayer()->Username},
                {"auto_fight", ActivePlayer()->AutoFight}
            }
        },
        {"turn", turnIndex},
        {"start_time", log.StartTime},
        {"players", pj},
        {"log", log.AsJson()}
    };
}


bool Fight::TryTakeTurn(const PlayerSession *initiator, const int atkMonID, const int defMonID, std::string *err) {
    if (winner) {
        if (err) *err = "Fight is already over.";
        return false;
    }
    if (ActivePlayer() != initiator) {
        if (err) *err = "It is NOT your turn (yet).";
        return false;
    }
    Monster *attacker = nullptr, *defender = nullptr;

    for (const auto p: players)
        for (const auto m: p->Monsters) {
            if (!m || !m->CheckIsAlive()) continue;
            if (m->ID == atkMonID && p == ActivePlayer())attacker = m;
            if (m->ID == defMonID)defender = m;
        }
    if (!attacker || !defender) {
        if (err) *err = "Not all monsters exist and or are capable to fight.";
        return false;
    }
    ExecuteTurn({attacker, defender});
    return true;
}

Fight::~Fight() {
    for (const auto p: players)
        for (const auto m: p->Monsters)
            if (m) m->LogPtr = nullptr;
}


void Fight::ExecuteTurn(MonTuple mons) {
    do {
        mons.attacker->Attack(mons.defender);
        turnIndex++;
        UpdateWinner();
        if (winner) return;
        if (!ActivePlayer()->AutoFight)return;
        mons = AutoPickMons();
    } while (mons.attacker && mons.defender);
}


Fight::MonTuple Fight::AutoPickMons() const {
    //Attacking Monster
    int ran = std::rand() % Config::Players::TeamSize;
    Monster *attacker = nullptr;
    for (int i = 0; i < Config::Players::TeamSize; i++) {
        attacker = ActivePlayer()->Monsters[(ran + i) % Config::Players::TeamSize];
        if (attacker && attacker->CheckIsAlive()) break;
    }
    if (!attacker || !attacker->CheckIsAlive()) return {nullptr, nullptr};

    //Targeted Player
    const PlayerSession *targetPlayer = nullptr;
    if (!attacker->IsHealer) {
        ran = std::rand() % fightSize;
        for (int i = 0; i < fightSize; i++) {
            targetPlayer = players[(ran + i) % fightSize];
            if (targetPlayer && targetPlayer != ActivePlayer()) break;
        }
    } else targetPlayer = ActivePlayer();
    if (!targetPlayer) return {nullptr, nullptr};

    //Targeted Monster
    ran = std::rand() % Config::Players::TeamSize;
    Monster *defender = nullptr;
    for (int i = 0; i < Config::Players::TeamSize; i++) {
        defender = targetPlayer->Monsters[(ran + i) % Config::Players::TeamSize];
        if (defender && defender->CheckIsAlive()) break;
    }
    if (!defender || !defender->CheckIsAlive()) return {nullptr, nullptr};
    return {attacker, defender};
}

void Fight::UpdateWinner() {
    PlayerSession *current = nullptr;
    for (const auto p: players)
        for (const auto m: p->Monsters) {
            if (!m || !m->CheckIsAlive()) continue;
            if (current && current != p) return;
            current = p;
        }
    winner = current;
}
