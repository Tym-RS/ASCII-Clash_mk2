#ifndef ASCII_CLASH_ORC_H
#define ASCII_CLASH_ORC_H

#include "../MonsterBase.h"
#include "../../MathHelpers.h"

class Orc final : public TypedMonster<MonsterType::Orc> {
public:
    using TypedMonster::TypedMonster;

protected:
    void AttackImpl(Monster *target) override {
        BaseReceiveAttack(target);
        const float headbuttRatio = SoftRatio(GetStatDict()->Get(Stat::Special), specialCounter);
        TryLog(Name + " has a " + std::to_string(static_cast<int>(headbuttRatio)) + "% headbutt chance.", LType::Nerdy);
        if (headbuttRatio < RandomPCT()) return;
        TryLog(Name + " headbutts " + target->Name + ", hurting both!", LType::Major);
        TakeDamage(headbuttDamage);
        target->TakeDamage(headbuttDamage * 2);
    }

private:
    static constexpr float specialCounter = 12.f;
    static constexpr int headbuttDamage = 1;
};

#endif
