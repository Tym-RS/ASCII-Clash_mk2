#include "Fight.h"
#include <string>
#include <nlohmann/json.hpp>
#include "Database/TableDefs.h"
#include "GameLogic/Monsters/Monsterbase.h"
#include "GameLogic/Team.h"

#define JStr(at) COL(Fights, at)

inline int GetHash(const std::vector<std::shared_ptr<Team> > &teams) {
    std::size_t h = 0;
    for (const auto &p: teams)
        h ^= (std::hash<std::string>{}(p->Name) << 1);
    h ^= time(nullptr);
    return static_cast<int>(h);
}


Fight::Fight(const std::vector<std::shared_ptr<Team> > &teams)
    : ID(GetHash(teams)), fightSize(teams.size()), teams(teams) {
    for (const auto &t: teams) t->EnterFight(&log);
    turnIndex = std::rand() % fightSize;
    log.Append(ActiveTeam()->Name + " starts!", LType::Major);
}

nlohmann::json Fight::ToJson() const {
    nlohmann::json wj = nullptr;
    if (winner)
        wj = {
            {"Name", winner->Name},
            {"ID", winner->ID}
        };
    auto participants = nlohmann::json::array();
    for (const auto &t: teams) if (t) participants.push_back(t->ToJson());

    return {
        {JStr(ID), ID},
        {JStr(is_ongoing), (winner == nullptr)},
        {JStr(winner), wj},
        {JStr(active_team), ActiveTeam()->Name},
        {JStr(turn), turnIndex},
        {JStr(log), log.ToJson()},
        {JStr(participants), participants}
    };
}


Fight Fight::FromJson(const nlohmann::json &j, const std::vector<std::shared_ptr<Team> > &teams) {
    auto fight = Fight(teams);
    fight.turnIndex = j.value("turn", 0);
    if (j.contains("winner") && !j["winner"].empty()) {
        const int winnerID = j["winner"]["ID"].get<int>();
        for (const auto &t: teams) {
            if (t->ID != winnerID) continue;
            fight.winner = t.get();
            break;
        }
    }
    if (j.contains("log")) fight.log = NestedLogger::FromJson(j["log"]);
    return fight;
}


bool Fight::TryTakeTurn(const Team *initiator, const int atkMonID, const int defMonID, std::string *err) {
    if (winner) {
        if (err) *err = "Fight is already over.";
        return false;
    }
    if (ActiveTeam() != initiator) {
        if (err) *err = "It is NOT your turn (yet).";
        return false;
    }
    Monster *attacker = nullptr, *defender = nullptr;

    for (const auto &t: teams)
        for (auto &m: t->Monsters()) {
            if (!m || !m->CheckIsAlive()) continue;
            if (m->ID == atkMonID && t.get() == ActiveTeam())attacker = m.get();
            if (m->ID == defMonID)defender = m.get();
        }
    if (!attacker || !defender) {
        if (err) *err = "Not all monsters exist and or are capable to fight.";
        return false;
    }
    ExecuteTurn({attacker, defender});
    return true;
}

Fight::~Fight() {
    for (const auto &t: teams) t->ExitFight();
}


void Fight::ExecuteTurn(TurnPair mons) {
    do {
        log.Next(mons.attacker->Name + " attacks " + mons.defender->Name, LType::Major);
        mons.attacker->Attack(mons.defender);

        for (const auto &team: teams) {
            int alive = 0, total = 0, totalHP = 0, totalMaxHP = 0;
            for (auto &m: team->Monsters()) {
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
            for (auto &m: team->Monsters()) {
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
        if (turnIndex >= Config::Fight::maxTurnCount) {
            log.Append("Draw — the fight timed out.", LType::Major);
            EndFight();
            return;
        }
        if (!ActiveTeam()->AutoFight) return;
        mons = AutoPickMons();
    } while (mons.attacker && mons.defender);
}


Fight::TurnPair Fight::AutoPickMons() const {
    //Attacking Monster
    int ran = std::rand() % Config::Team::Size;
    Monster *attacker = nullptr;
    for (int i = 0; i < Config::Team::Size; i++) {
        attacker = ActiveTeam()->Monsters()[(ran + i) % Config::Team::Size].get();
        if (attacker && attacker->CheckIsAlive()) break;
    }
    if (!attacker || !attacker->CheckIsAlive()) return {nullptr, nullptr};

    //Targeted Team
    const Team *targetPlayer = nullptr;
    if (!attacker->IsHealer()) {
        ran = std::rand() % fightSize;
        for (int i = 0; i < fightSize; i++) {
            targetPlayer = teams[(ran + i) % fightSize].get();
            if (targetPlayer && targetPlayer != ActiveTeam()) break;
        }
    } else targetPlayer = ActiveTeam();
    if (!targetPlayer) return {nullptr, nullptr};

    //Defending Monster
    ran = std::rand() % Config::Team::Size;
    Monster *defender = nullptr;
    for (int i = 0; i < Config::Team::Size; i++) {
        defender = targetPlayer->Monsters()[(ran + i) % Config::Team::Size].get();
        if (defender && defender->CheckIsAlive()) break;
    }
    if (!defender || !defender->CheckIsAlive()) return {nullptr, nullptr};
    return {attacker, defender};
}

void Fight::UpdateWinner() {
    Team *current = nullptr;
    for (const auto &t: teams)
        for (const auto &m: t->Monsters()) {
            if (!m || !m->CheckIsAlive()) continue;
            if (current && current != t.get()) return;
            current = t.get();
        }
    if (!current) return;
    winner = current;
    EndFight();
}

void Fight::EndFight() const {
    int expGain = 0;
    for (const auto &t: teams) {
        if (t.get() == winner)continue;
        for (const auto &m: t->Monsters())
            if (m) expGain += m->GetStatDict()->Get(Stat::Level);
        t->ExitFight();
    }
    if (winner) winner->ExitFight(expGain);
}


#undef JStr
