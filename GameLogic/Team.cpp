#include "Team.h"

#include <iostream>
#include <utility>
#include "Database/TableDefs.h"
#include "Monsters/CreateMonster.h"

#define JStr(at) COL(Teams, at)

Team::Team(std::string name, const int id) : Name(std::move(name)), ID(id) {
}

Team Team::FromJson(const nlohmann::json &data) {
    auto team = Team(data[JStr(name)].get<std::string>(), data[JStr(ID)].get<int>());
    team.currentFightID = data.value(JStr(fight_id), -1);
    team.AutoFight = data.value(JStr(auto_fight), false);
    const auto monsterData = data.value(JStr(monsters), nlohmann::json::array());
    for (int i = 0; i < Config::Team::Size; i++)
        if (i >= monsterData.size() || monsterData[i].is_null()) team.monsters[i] = nullptr;
        else team.monsters[i] = Monster::FromJson(monsterData[i]);

    return team;
}

nlohmann::json Team::ToJson() const {
    nlohmann::json mons = nlohmann::json::array();
    for (const auto &m: monsters) mons.push_back(m ? m->ToJson() : nullptr);
    return {
        {JStr(ID), ID},
        {JStr(name), Name},
        {JStr(in_fight), IsInFight()},
        {JStr(fight_id), currentFightID},
        {JStr(monsters), mons},
        {JStr(auto_fight), AutoFight}
    };
}

int Team::GetAverageLvl() const {
    int lvl = 0;
    for (auto &m: monsters)
        if (m)lvl += m->GetStatDict()->Get(Stat::Level);
    return lvl == 0 ? 0 : lvl / monsters.size();
}

int Team::TryGetNewMonsterID(const std::string &name, const MonsterType type, std::string *err) {
    int emptySlot = -1;
    for (int i = 0; i < Config::Team::Size; i++) {
        const auto m = monsters[i].get();
        if (!m && emptySlot == -1) emptySlot = i;
        if (m && m->Name == name) {
            SET_ERR("A monster with the same name already exists.");
            return -1;
        }
    }
    if (emptySlot == -1) {
        SET_ERR("No empty slots in team.");
        return -1;
    }
    auto mon = CreateMonster(name, ID * Config::Team::Size + emptySlot, type);
    const int newID = mon->ID;
    monsters[emptySlot] = std::move(mon);
    return newID;
}

Monster *Team::TryGetMonster(const int id, std::string *err) const {
    for (const auto &m: monsters)
        if (m && m->ID == id) return m.get();
    SET_ERR("Monster not found.");
    return nullptr;
}

bool Team::TryDeleteMonster(const int id, std::string *err) {
    if (IsInFight()) {
        SET_ERR("CanNOT delete monsters while in a fight.");
        return false;
    }
    for (auto &m: monsters) {
        if (!m || m->ID != id) continue;
        m = nullptr;
        return true;
    }
    SET_ERR("Monster not found.");
    return false;
}


void Team::EnterFight(const int fightID, NestedLogger *log) {
    if (IsInFight() && currentFightID != fightID) {
        std::cerr << "TEAM " << ID << " tried to enter fight [" << fightID << "] while already being in [" <<
                currentFightID << "]" << std::endl;
        return;
    }
    for (const auto &m: monsters) {
        if (!m) continue;
        m->LogPtr = log;
    }
    currentFightID = fightID;
}

void Team::ExitFight(const int expGain) {
    if (!IsInFight()) return;
    const int expPerMon = expGain > 0 ? expGain / Config::Team::Size + 1 : 0;
    for (auto &m: monsters) {
        if (m && !m->CheckIsAlive()) m = nullptr;
        if (!m) continue;
        m->GetStatDict()->ReceiveEXP(expPerMon);
        m->LogPtr = nullptr;
        m->Reset();
    }
    currentFightID = -1;
}

#undef JStr
