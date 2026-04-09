#include "../Monsters.h"
#include "../MathHelpers.h"

bool Methog::ReceiveAttack(Monster *from) {
    if (CalculateHitChance(from->GetStatDict().Get(Stat::Offense), GetStatDict().Get(Stat::Defense)) > RandomPCT()) {
        TryLog(Name + " has dodged.", LogType::event);
        return false;
    }
    TryLog(Name + " was hit.", LogType::event);
    TakeDamage(from->GetStatDict().Get(Stat::Damage));
    from->TakeDamage(from->GetStatDict().Get(Stat::Damage) / 2);
    return true;
}
