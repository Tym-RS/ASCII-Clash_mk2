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


#endif
