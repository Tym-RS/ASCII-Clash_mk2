#ifndef ASCII_CLASH_STATS_H
#define ASCII_CLASH_STATS_H
#include "Config.h"
#include <unordered_map>


enum class Stat {
#define X(stat, ...) stat,
    MONSTER_STATS
#undef X
    COUNT
};

struct StatInfo {
    constexpr StatInfo(const char *statString, const char *description,
                       const int defaultValue, const int levelUpAmount) : AsString(statString),
                                                                          Description(description),
                                                                          Levelable(levelUpAmount > 0),
                                                                          LevelUpAmount(levelUpAmount),
                                                                          DefaultValue(defaultValue) {
    }

    const char *AsString, *Description;
    const bool Levelable;
    const int LevelUpAmount, DefaultValue;
};


constexpr inline StatInfo StatInfos[] = {
#define X(type, desc, dflt, lvlAmnt) StatInfo(#type, desc, dflt, lvlAmnt),
    MONSTER_STATS
#undef X
};


const inline std::unordered_map<std::string, Stat> StringStatMap = {
#define X(stat, ...) {#stat, Stat::stat},
    MONSTER_STATS
#undef X
};

#endif
