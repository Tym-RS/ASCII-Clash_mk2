#ifndef ASCII_CLASH_MATHHELPERS_H
#define ASCII_CLASH_MATHHELPERS_H
#include "../../Config.h"

inline float SoftRatio(const int value, const int counter) { return static_cast<float>(value) / (counter + value); }


inline float CalculateHitChance(const int attack, const int defense) {
    const float attackBonus = SoftRatio(attack, Config::Monsters::AttackStatCounter);
    const float defenseBonus = SoftRatio(defense, Config::Monsters::DefenseStatCounter);
    const float chance = Config::Monsters::BaseHitChance + attackBonus - defenseBonus;
    return chance > Config::Monsters::MinHitChance ? chance : Config::Monsters::MinHitChance;
}

inline float RandomPCT() { return static_cast<float>(rand()) / RAND_MAX; }


#endif
