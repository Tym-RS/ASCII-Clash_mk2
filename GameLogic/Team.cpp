#include "Team.h"
#include <utility>
#include "Database/TableDefs.h"

#define JStr(at) COL(Teams, at)

Team::Team(std::string name, const int id) : Name(std::move(name)), ID(id) {
}

Team Team::FromJson(const nlohmann::json &j) {
    auto t = Team(j[JStr(name)].get<std::string>(), j[JStr(ID)].get<int>());
    const auto monJ = j[JStr(monsters)];
    for (int i = 0; i < Config::Team::Size; i++) {
        if (monJ[i].is_null()) t.monsters[i] = nullptr;
        else t.monsters[i] = Monster::FromJson(monJ[i]);
    }
    return t;
}

nlohmann::json Team::ToJson() const {
    nlohmann::json mons = nlohmann::json::array();
    for (const auto &m: monsters) mons.push_back(m ? m->ToJson() : nullptr);
    return {
        {JStr(ID), ID},
        {JStr(name), Name},
        {JStr(monsters), mons},
    };
}

void Team::EnterFight(NestedLogger *l) {
    for (auto &m: monsters)
        if (m) m->LogPtr = l;
    isInFight = true;
}

void Team::ExitFight(const int expGain) {
    if (!isInFight) return;
    const int expPerMon = expGain > 0 ? expGain / Config::Team::Size : 0;
    for (auto &m: monsters) {
        if (m && !m->CheckIsAlive()) m = nullptr;
        if (!m) continue;
        m->GetStatDict()->ReceiveEXP(expPerMon);
        m->LogPtr = nullptr;
        m->Reset();
    }
    isInFight = false;
}

#undef JStr
