#ifndef ASCII_CLASH_STRINGS_H
#define ASCII_CLASH_STRINGS_H

#include "Config.h"
#include <unordered_map>
#include <string>
#include <utility>
#include <nlohmann/json.hpp>

enum class MonsterType {
#define X(type, desc) type,
    MONSTER_TYPES
#undef X
};


struct MonDescription {
    MonDescription(std::string typeName, std::string description) : TypeAsString(std::move(typeName)),
                                                                    Description(std::move(description)) {
    }

    const std::string TypeAsString, Description;
};

const inline std::unordered_map<MonsterType, MonDescription> MonsterDescriptions = {
#define X(type, desc) { MonsterType::type, MonDescription(#type, desc) },
    MONSTER_TYPES
#undef X
};

const inline std::unordered_map<std::string, MonsterType> MonsterTypeStringMap = {
#define X(type, ...) { #type, MonsterType::type },
    MONSTER_TYPES
#undef X
};


inline nlohmann::json GetGameDescriptionsJSON() {
    return nlohmann::json{
#define X(type, desc) {#type, desc},
        {
            "Monsters", nlohmann::json{
                MONSTER_TYPES
            }
        },
#undef X
#define X(stat, desc, dflt, lvlAmnt) {#stat, {{"Description", desc}, {"Default", dflt}, {"LevelUp", lvlAmnt}}},
        {
            "Stats", nlohmann::json{
                MONSTER_STATS
            }
        }
#undef X
    };
}


#endif
