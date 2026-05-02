#pragma once

#include "../MonsterBase.h"

class Methog final : public TypedMonster<MonsterType::Methog> {
public:
    using TypedMonster::TypedMonster;

protected:
    bool ReceiveAttackImpl(Monster *from) override {
        const float hitChance = CalculateHitChance(from->GetStatDict()->Get(Stat::Offense),
                                                   GetStatDict()->Get(Stat::Defence));
        TryLog(Name + " has a " + std::to_string(hitChance * 100.0) + "% chance to dodge.", LType::Nerdy);
        if (hitChance > RandomPCT()) {
            TryLog(Name + " sidesteps the blow!", LType::Major);
            return false;
        }
        TryLog(Name + " is struck!", LType::Major);
        const int damage = from->GetStatDict()->Get(Stat::Damage);
        TakeDamage(damage);
        TryLog(Name + " spikes hurt, dealing " + std::to_string(damage / 2) + " damage back!", LType::Major);
        from->TakeDamage(damage / 2);
        return true;
    }
};
