#include "../Monsters.h"
#include "../MathHelpers.h"

inline constexpr float specialCounter = 15;

bool Human::ReceiveAttack(Monster *from) {
    if (Monster::ReceiveAttack(from)) return true;
    if (SoftRatio(GetStatDict().Get(Stat::Special), specialCounter) >= RandomPCT()) {
        TryLog(Name + " performs a riposte!", LogType::event);
        Attack(from);
    }
    return false;
}
