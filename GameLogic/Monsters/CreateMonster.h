#ifndef ASCII_CLASH_MONSTERFACTORY_H
#define ASCII_CLASH_MONSTERFACTORY_H

#include "MonsterClasses/Human.h"
#include "MonsterClasses/Orc.h"
#include "MonsterClasses/Methog.h"
#include "MonsterClasses/Ratkin.h"
#include "Monsterbase.h"
#include "Config.h"


inline std::unique_ptr<Monster> CreateMonster(
    const std::string &name, const int id, const MonsterType type,
    const StatDict *stats = nullptr) {
    switch (type) {
#define X(type, ...) case MonsterType::type: return stats ? std::make_unique<type>(name, id, *stats) : std::make_unique<type>(name, id);
        MONSTER_TYPES
#undef X
        default:
            return stats
                       ? std::make_unique<Monster>(name, id, MonsterType::None, *stats)
                       : std::make_unique<Monster>(name, id, MonsterType::None);
    }
}


inline nlohmann::json Monster::ToJson() const {
    return nlohmann::json{
        {"ID", ID},
        {"name", Name},
        {"type", MonsterDescriptions.at(Type).TypeAsString},
        {"is_healer", IsHealer()},
        {"current_health", currentHealth},
        {"healing_done", healingDone},
        {"stats", stats.ToJson()},
    };
}

inline std::unique_ptr<Monster> Monster::FromJson(const nlohmann::json &j) {
    const MonsterType type = MonsterTypeStringMap.at(j["type"].get<std::string>());
    const auto stats = StatDict::FromJson(j["stats"]);
    auto m = CreateMonster(j["name"].get<std::string>(), j["ID"].get<int>(), type, &stats);
    m->currentHealth = j["current_health"].get<int>();
    m->healingDone = j["healing_done"].get<int>();
    return m;
}

#endif
