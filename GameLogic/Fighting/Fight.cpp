#include "Fight.h"
#include <string>
#include <cstdlib>

Fight::Fight(const std::array<Team *, fightSize> players)
    : ID([&] {
        std::size_t h = 0;
        for (const auto p: players)
            h ^= (std::hash<std::string>{}(p->) << 1);
        return std::to_string(h);
    }()), teams(players) {
    std::string startMessage = "Fight between ";
    for (size_t i = 0; i < fightSize; ++i) {
        const auto *p = players[i];
        startMessage += p->Name;
        if (i + 1 < players.size()) startMessage += " and ";
        for (auto *m: p->Monsters()) if (m) m->LogPtr = &log;
    }
    startMessage += ".";
    log.Append(startMessage, LType::Minor);

    turnIndex = std::rand() % fightSize;
    log.Append(ActiveTeam()->Name + " starts!", LType::Major);
}

nlohmann::json Fight::ToJson() {
    nlohmann::json pj = nlohmann::json::array();
    for (const auto p: teams) {
        int aliveCount = 0, totalCurrentHP = 0;
        for (const auto m: p->Monsters()) {
            if (!m || !m->CheckIsAlive()) continue;
            aliveCount++;
            totalCurrentHP += m->GetCurrentHealth();
        }
        pj.push_back({
        });
    }

    return {
        {"id", ID},
        {"is_ongoing", !winner},
        {"winner", winner ? winner->Name : nullptr},
        {
            "active_team", {
                {"name", ActiveTeam()->Name},
                {"auto_fight", ActiveTeam()->AutoFight}
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
    if (ActiveTeam() != initiator) {
        if (err) *err = "It is NOT your turn (yet).";
        return false;
    }
    Monster *attacker = nullptr, *defender = nullptr;

    for (const auto p: players)
        for (const auto m: p->Monsters) {
            if (!m || !m->CheckIsAlive()) continue;
            if (m->ID == atkMonID && p == ActiveTeam())attacker = m;
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
        log.Next(mons.attacker->Name + " attacks " + mons.defender->Name, LType::Major);
        mons.attacker->Attack(mons.defender);

        // Team status snapshot after each attack
        for (auto *team: teams) {
            int alive = 0, total = 0, totalHP = 0, totalMaxHP = 0;
            for (auto *m: team->Monsters()) {
                if (!m) continue;
                total++;
                totalMaxHP += m->GetStatDict()->Get(Stat::Health);
                if (m->CheckIsAlive()) {
                    alive++;
                    totalHP += m->GetCurrentHealth();
                }
            }
            log.Append(team->Name + ": " + std::to_string(alive) + "/" + std::to_string(total) +
                       " standing, " + std::to_string(totalHP) + "/" + std::to_string(totalMaxHP) + " HP total",
                       LType::Major);
            for (auto *m: team->Monsters()) {
                if (!m) continue;
                log.Append(m->Name + ": " + std::to_string(m->GetCurrentHealth()) + "/" +
                           std::to_string(m->GetStatDict()->Get(Stat::Health)) + " HP", LType::Nerdy);
            }
        }

        turnIndex++;
        UpdateWinner();
        if (winner) {
            log.Append(winner->Name + " wins!", LType::Major);
            return;
        }
        if (turnIndex >= 100) {
            log.Append("Draw — the fight timed out after 100 turns.", LType::Major);
            return;
        }
        if (!ActiveTeam()->AutoFight) return;
        mons = AutoPickMons();
    } while (mons.attacker && mons.defender);
}


Fight::MonTuple Fight::AutoPickMons() const {
    //Attacking Monster
    int ran = std::rand() % Config::Team::Size;
    Monster *attacker = nullptr;
    for (int i = 0; i < Config::Team::Size; i++) {
        attacker = ActiveTeam()->Monsters[(ran + i) % Config::Team::Size];
        if (attacker && attacker->CheckIsAlive()) break;
    }
    if (!attacker || !attacker->CheckIsAlive()) return {nullptr, nullptr};

    //Targeted Player
    const PlayerSession *targetPlayer = nullptr;
    if (!attacker->IsHealer()) {
        ran = std::rand() % fightSize;
        for (int i = 0; i < fightSize; i++) {
            targetPlayer = players[(ran + i) % fightSize];
            if (targetPlayer && targetPlayer != ActiveTeam()) break;
        }
    } else targetPlayer = ActiveTeam();
    if (!targetPlayer) return {nullptr, nullptr};

    //Defending Monster
    ran = std::rand() % Config::Team::Size;
    Monster *defender = nullptr;
    for (int i = 0; i < Config::Team::Size; i++) {
        defender = targetPlayer->Monsters[(ran + i) % Config::Team::Size];
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
