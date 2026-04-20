#ifndef ASCII_CLASH_MONSTERFACTORY_H
#define ASCII_CLASH_MONSTERFACTORY_H

#include "MonsterClasses/Human.h"
#include "MonsterClasses/Orc.h"
#include "MonsterClasses/Methog.h"
#include "MonsterClasses/Ratkin.h"
#include "Monsterbase.h"
#include "Config.h"

inline Monster *CreateTypedMonster(const std::string &name, const int id, const MonsterType type,
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

#endif