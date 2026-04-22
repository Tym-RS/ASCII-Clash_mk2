#ifndef ASCII_CLASH_MATHHELPERS_H
#define ASCII_CLASH_MATHHELPERS_H
#include "../Config.h"

inline float SoftRatio(const int value, const int counter) { return static_cast<float>(value) / (counter + value); }


inline float CalculateHitChance(const int attack, const int defense) {
    const float attackBonus = SoftRatio(attack, Config::Monster::AttackStatCounter);
    const float defenseBonus = SoftRatio(defense, Config::Monster::DefenseStatCounter);
    const float chance = Config::Monster::BaseHitChance + attackBonus - defenseBonus;
    return chance > Config::Monster::MinHitChance ? chance : Config::Monster::MinHitChance;
}

inline float RandomPCT() { return static_cast<float>(rand()) / RAND_MAX; }


#endif
