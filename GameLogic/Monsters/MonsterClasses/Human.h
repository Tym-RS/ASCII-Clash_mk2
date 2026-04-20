#ifndef ASCII_CLASH_HUMAN_H
#define ASCII_CLASH_HUMAN_H

#include "../Monsterbase.h"
#include "../MathHelpers.h"

class Human final : public TypedMonster<MonsterType::Human> {
public:
    using TypedMonster::TypedMonster;

protected:
    bool ReceiveAttackImpl(Monster *from) override {
        if (Monster::ReceiveAttackImpl(from)) return true;

        if (SoftRatio(GetStatDict()->Get(Stat::Special), specialCounter) >= RandomPCT()) {
            TryLog(Name + " performs a riposte!", LType::Major);
            Attack(from);
        }
        return false;
    }

private:
    static constexpr float specialCounter = 15.f;
};

#endif
