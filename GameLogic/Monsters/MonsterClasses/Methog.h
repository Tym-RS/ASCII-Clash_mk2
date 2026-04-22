#ifndef ASCII_CLASH_METHOG_H
#define ASCII_CLASH_METHOG_H

#include "../Monsterbase.h"
#include "../../MathHelpers.h"

class Methog final : public TypedMonster<MonsterType::Methog> {
public:
    using TypedMonster::TypedMonster;

protected:
    bool ReceiveAttackImpl(Monster *from) override {
        if (CalculateHitChance(from->GetStatDict()->Get(Stat::Offense), GetStatDict()->Get(Stat::Defence)) >
            RandomPCT()) {
            TryLog(Name + " has dodged.", LType::Major);
            return false;
        }
        TryLog(Name + " was hit.", LType::Major);
        const int damage = from->GetStatDict()->Get(Stat::Damage);
        TakeDamage(damage);
        from->TakeDamage(damage / 2);
        return true;
    }
};

#endif
