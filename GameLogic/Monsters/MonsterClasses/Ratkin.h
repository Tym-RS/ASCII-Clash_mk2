#pragma once

#include "../MonsterBase.h"

class Ratkin final : public TypedMonster<MonsterType::Ratkin> {
public:
    using TypedMonster::TypedMonster;

    // nlohmann::json ToJsonImpl() const override;
    //
    // void FromJsonImpl(const nlohmann::json &j) override;
    //
    // bool ReceiveAttackImpl(Monster *from) override;
    //
    // void AttackImpl(Monster *target) override;

private:
    float dodgeAddition = 0;
};
