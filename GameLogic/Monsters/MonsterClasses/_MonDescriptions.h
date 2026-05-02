#pragma once

#include <unordered_map>
#include <string>
#include <utility>
#include <nlohmann/json.hpp>


#define MONSTER_TYPES \
X(Human,  "Average human. Can counter attack.") \
X(Orc,    "A big GREEN savage. Can headbutt.") \
X(Methog, "Hedgehog, but with Metal. Touch = Hurt") \
X(Ratkin, "A stealth-sneak rat. Ugly like you.")


enum class MonsterType {
#define X(type, desc, ...) type,
    MONSTER_TYPES
#undef X
    COUNT
};


struct MonDescription {
    MonDescription(std::string typeName, std::string description) : TypeAsString(std::move(typeName)),
                                                                    Description(std::move(description)) {
    }

    const std::string TypeAsString, Description;
};

const inline std::unordered_map<MonsterType, MonDescription> MonsterDescriptions = {
#define X(type, desc, ...) { MonsterType::type, MonDescription(#type, desc) },
    MONSTER_TYPES
#undef X
};

const inline std::unordered_map<std::string, MonsterType> StringMonsterTypeMap = {
#define X(type, ...) { #type, MonsterType::type },
    MONSTER_TYPES
#undef X
};


inline nlohmann::json GetGameDescriptionsJSON() {
    return nlohmann::json{
#define X(type, desc, ...) {#type, desc},
        {
            "Monsters", nlohmann::json{
                MONSTER_TYPES
            }
        },
#undef X

#define X(stat, desc, dflt, lvlAmnt, ...) {#stat, {{"Description", desc}, {"Default", dflt}, {"LevelUp", lvlAmnt}}},
        {
            "Stats", nlohmann::json{
                MONSTER_STATS
            }
        }
#undef X
    };
}
