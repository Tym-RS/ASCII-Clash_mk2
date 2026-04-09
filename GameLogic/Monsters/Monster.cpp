#include <utility>

#include "Monsters.h"
#include "MathHelpers.h"

Monster::Monster(std::string name, std::string id, const MonsterType type) : Name(std::move(name)), ID(std::move(id)),
                                                                             Type(type),
                                                                             currentHealth(GetStatDict().Get(Stat::Health)) {
}

bool Monster::IsAlive() const {
    return currentHealth > 0;
}

void Monster::Attack(Monster *target) {
    TryLog(Name + " attacks " + target->Name + ".", LogType::event);
    target->ReceiveAttack(this);
}

void Monster::TakeDamage(const int amount) {
    currentHealth -= amount;
    std::ranges::clamp(currentHealth, 0, GetStatDict().Get(Stat::Health));
    TryLog(Name + " takes " + std::to_string(amount) + " damage.", LogType::info);
}

void Monster::Heal(const int amount) {
    currentHealth += amount;
    std::ranges::clamp(currentHealth, 0, GetStatDict().Get(Stat::Health));
    TryLog(Name + " heals " + std::to_string(amount) + " HP.", LogType::info);
}

bool Monster::ReceiveAttack(Monster *from) {
    if (CalculateHitChance(from->GetStatDict().Get(Stat::Offense), GetStatDict().Get(Stat::Defense)) > RandomPCT()) {
        TryLog(Name + " has dodged.", LogType::event);
        return false;
    }
    TryLog(Name + " was hit.", LogType::event);
    TakeDamage(from->GetStatDict().Get(Stat::Damage));
    return true;
}

void Monster::TryLog(const std::string &message, const LogType type) const {
    if (LogPtr == nullptr) return;
    LogPtr->Log(message, type);
}
