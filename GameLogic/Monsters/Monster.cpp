#include <utility>

#include "Monsters.h"
#include "MathHelpers.h"

Monster::Monster(std::string name, const int id, const MonsterType type) : Name(std::move(name)),
                                                                           ID(id), Type(type),
                                                                           currentHealth(1) {
    currentHealth = GetStatDict()->Get(Stat::Health);
}

Monster::Monster(std::string name, const int id, const MonsterType type, const StatDict &stats) : Name(std::move(name)),
    ID(id), Type(type), currentHealth(stats.Get(Stat::Health)), stats(stats) {
}

bool Monster::IsAlive() const {
    return currentHealth > 0;
}

void Monster::Attack(Monster *target) {
    TryLog(Name + " attacks " + target->Name + ".", LType::Major);
    target->ReceiveAttack(this);
}

void Monster::TakeDamage(const int amount) {
    currentHealth -= amount;
    std::ranges::clamp(currentHealth, 0, GetStatDict()->Get(Stat::Health));
    TryLog(Name + " takes " + std::to_string(amount) + " damage.", LType::Minor);
}

void Monster::Heal(const int amount) {
    currentHealth += amount;
    std::ranges::clamp(currentHealth, 0, GetStatDict()->Get(Stat::Health));
    TryLog(Name + " heals " + std::to_string(amount) + " HP.", LType::Minor);
}

bool Monster::ReceiveAttack(Monster *from) {
    if (CalculateHitChance(from->GetStatDict()->Get(Stat::Offense), GetStatDict()->Get(Stat::Defense)) > RandomPCT()) {
        TryLog(Name + " has dodged.", LType::Major);
        return false;
    }
    TryLog(Name + " was hit.", LType::Major);
    TakeDamage(from->GetStatDict()->Get(Stat::Damage));
    return true;
}

void Monster::TryLog(const std::string &message, const LType type) const {
    if (LogPtr == nullptr) return;
    LogPtr->Append(message, type);
}
