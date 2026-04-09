#include "../Monsters.h"
#include "../MathHelpers.h"

inline constexpr float specialCounter = 12;
inline constexpr int headButtDamage = 1;

void Orc::Attack(Monster *target) {
    Monster::Attack(target);
    if (SoftRatio(GetStatDict().Get(Stat::Special), specialCounter) < RandomPCT()) return;
    TryLog(Name + " headbutts " + target->Name + ", dealing damage to both!", LogType::event);
    TakeDamage(headButtDamage);
    target->TakeDamage(headButtDamage * 2);
}
