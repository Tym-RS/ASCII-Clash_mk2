#ifndef ASCII_CLASH_ORC_H
#define ASCII_CLASH_ORC_H

#include "../Monsterbase.h"
#include "../MathHelpers.h"

class Orc final : public TypedMonster<MonsterType::Orc> {
public:
    using TypedMonster::TypedMonster;

protected:
    void AttackImpl(Monster *target) override {
        Monster::AttackImpl(target);
        if (SoftRatio(GetStatDict()->Get(Stat::Special), specialCounter) < RandomPCT()) return;
        TryLog(Name + " headbutts " + target->Name + ", dealing damage to both!", LType::Major);
        TakeDamage(headbuttDamage);
        target->TakeDamage(headbuttDamage * 2);
    }

private:
    static constexpr float specialCounter = 12.f;
    static constexpr int headbuttDamage = 1;
};

#endif
