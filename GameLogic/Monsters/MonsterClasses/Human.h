#ifndef ASCII_CLASH_HUMAN_H
#define ASCII_CLASH_HUMAN_H

#include "../MonsterBase.h"
#include "../../MathHelpers.h"

class Human final : public TypedMonster<MonsterType::Human> {
public:
    using TypedMonster::TypedMonster;

protected:
    bool ReceiveAttackImpl(Monster *from) override {
        if (BaseReceiveAttack(from)) return true;

        const float riposteChance = SoftRatio(GetStatDict()->Get(Stat::Special), specialCounter);
        TryLog(Name + " has a " + std::to_string(riposteChance * 100.0) + "% riposte chance.", LType::Nerdy);
        if (riposteChance >= RandomPCT()) {
            TryLog(Name + " strikes back with a riposte!", LType::Major);
            Attack(from);
        }
        return false;
    }

private:
    static constexpr float specialCounter = 15.f;
};

#endif
