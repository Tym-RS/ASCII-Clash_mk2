#ifndef ASCII_CLASH_METHOG_H
#define ASCII_CLASH_METHOG_H

#include "../MonsterBase.h"
#include "../../MathHelpers.h"

class Methog final : public TypedMonster<MonsterType::Methog> {
public:
    using TypedMonster::TypedMonster;

protected:
    bool ReceiveAttackImpl(Monster *from) override {
        const int hitChance = static_cast<int>(
            CalculateHitChance(from->GetStatDict()->Get(Stat::Offense), GetStatDict()->Get(Stat::Defence)));
        TryLog(Name + " has a " + std::to_string(hitChance) + "% chance to dodge.", LType::Nerdy);
        if (hitChance > RandomPCT()) {
            TryLog(Name + " sidesteps the blow!", LType::Major);
            return false;
        }
        TryLog(Name + " is struck!", LType::Major);
        const int damage = from->GetStatDict()->Get(Stat::Damage);
        TakeDamage(damage);
        TryLog(Name + " retaliates, dealing " + std::to_string(damage / 2) + " damage back!", LType::Major);
        from->TakeDamage(damage / 2);
        return true;
    }
};

#endif