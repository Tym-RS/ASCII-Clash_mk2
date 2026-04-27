#ifndef ASCII_CLASH_RATKIN_H
#define ASCII_CLASH_RATKIN_H

#include "../MonsterBase.h"

class Ratkin final : public TypedMonster<MonsterType::Ratkin> {
public:
    using TypedMonster::TypedMonster;
};

#endif
