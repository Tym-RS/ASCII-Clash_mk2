#ifndef ASCII_CLASH_MONSTERFACTORY_H
#define ASCII_CLASH_MONSTERFACTORY_H

#include "MonsterClasses/Human.h"
#include "MonsterClasses/Orc.h"
#include "MonsterClasses/Methog.h"
#include "MonsterClasses/Ratkin.h"
#include "Monsterbase.h"
#include "Config.h"


inline Monster *CreateMonster(const std::string &name, const int id, const MonsterType type,
                              const StatDict *stats = nullptr) {
    switch (type) {
#define X(type, ...) case MonsterType::type: return stats ? new type(name, id, *stats) : new type(name, id);
        MONSTER_TYPES
#undef X
        default:
            return stats
                       ? new Monster(name, id, MonsterType::None, *stats)
                       : new Monster(name, id, MonsterType::None);
    }
}


inline nlohmann::json Monster::ToJson() {
    return nlohmann::json{
        {"ID", ID},
        {"Name", Name},
        {"Type", MonsterDescriptions.at(Type).TypeAsString},
        {"IsHealer", IsHealer()},
        {"CurrentHealth", currentHealth},
        {"Stats", stats.ToJson()}
    };
}

inline Monster *Monster::FromJson(const nlohmann::json &j) {
    const MonsterType type = MonsterTypeStringMap.at(j["Type"].get<std::string>());
    const StatDict *stats = StatDict::FromJson(j["Stats"]);
    Monster *m = CreateMonster(j["Name"].get<std::string>(), j["ID"].get<int>(), type, stats);
    m->currentHealth = j["CurrentHealth"].get<int>();
    delete stats;
    return m;
}

#endif
