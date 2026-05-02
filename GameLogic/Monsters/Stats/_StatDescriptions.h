#pragma once

#include <unordered_map>
#include <string>


#define MONSTER_STATS \
X(Damage,  "How much damage each hit from this monster deals.", 1, 2) \
X(Offense, "How likely it is, for an attack from this monster to land.", 1, 1) \
X(Defence, "How likely it is, that this monster dodges an incoming attack.", 1, 1) \
X(Special, "Different for each monster. Refer to its description.", 1, 2) \
X(Health,  "The maximum HP this monster can have.", 10, 2) \
X(Exp,     "How much EXP this monster has. (It needs [2*Level] to level up.)", 0, 0) \
X(Level,   "The current level of this monster. (This times two is how much EXP it needs to gain a new level.)", 1, 0) \
X(SkillPoints, "Each skill-point can level one stat of it's monster. A new point is gained on level-up.", 3, 0)


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
