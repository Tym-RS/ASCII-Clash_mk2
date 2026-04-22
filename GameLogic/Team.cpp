#include "Team.h"

#include <utility>
#include "Database/TableDefs.h"


#define JStr(at) COL(Teams, at)

Team::Team(std::string name, const int id) : Name(std::move(name)), ID(id) {
}

Team *Team::FromJson(const nlohmann::json &j) {
    const auto t = new Team(j[JStr(Name)].get<std::string>(), j[JStr(ID)].get<int>());
    auto monJ = j[JStr(MonsterIDs)];
    for (int i = 0; i < Config::Team::Size; i++) {
        if (monJ[i].is_null()) t->monsters[i] = nullptr;
        else t->monsters[i] = Monster::FromJson(monJ[i]);
    }
    return t;
}

nlohmann::json Team::ToJson() {
    nlohmann::json mons = nlohmann::json::array();
    for (const auto m: monsters) mons.push_back(m ? m->ToJson() : nullptr);
    return {
        {JStr(ID), ID},
        {JStr(Name), Name},
        {JStr(MonsterIDs), mons},
    };
}

Team::~Team() {
    for (const auto m: monsters) delete m;
}
