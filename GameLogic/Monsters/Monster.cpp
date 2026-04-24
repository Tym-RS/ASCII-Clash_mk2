#include <algorithm>
#include <utility>

#include "CreateMonster.h"
#include "GameLogic/Monsters/Monsterbase.h"
#include "../MathHelpers.h"

Monster::Monster(std::string name, const int id, const MonsterType type)
    : ID(id), Name(std::move(name)), Type(type) {
    currentHealth = GetStatDict()->Get(Stat::Health);
}

Monster::Monster(std::string name, const int id, const MonsterType type, StatDict stats)
    : ID(id), Name(std::move(name)), Type(type), stats(std::move(stats)) {
    currentHealth = GetStatDict()->Get(Stat::Health);
}


nlohmann::json Monster::ToJson() const {
    return nlohmann::json{
        {"ID", ID},
        {"name", Name},
        {"type", MonsterDescriptions.at(Type).TypeAsString},
        {"is_healer", IsHealer()},
        {"current_health", currentHealth},
        {"healing_done", healingDone},
        {"stats", stats.ToJson()},
    };
}

std::unique_ptr<Monster> Monster::FromJson(const nlohmann::json &j) {
    const MonsterType type = MonsterTypeStringMap.at(j["type"].get<std::string>());
    const auto stats = StatDict::FromJson(j["stats"]);
    auto m = CreateMonster(j["name"].get<std::string>(), j["ID"].get<int>(), type, &stats);
    m->currentHealth = j["current_health"].get<int>();
    m->healingDone = j["healing_done"].get<int>();
    return m;
}

void Monster::Reset() {
    currentHealth = GetStatDict()->Get(Stat::Health);
    healingDone = 0;
    ResetImpl();
}

void Monster::Attack(Monster *target) {
    if (!CheckIsAlive() || !target->CheckIsAlive()) return;
    AttackImpl(target);
}

void Monster::TakeDamage(const int amount) {
    if (!CheckIsAlive()) return;
    TakeDamageImpl(amount);
    if (currentHealth <= 0) {
        currentHealth = 0;
        OnDeath();
    }
}

void Monster::Heal(int amount) {
    if (!CheckIsAlive()) return;

    const int maxHealth = static_cast<int>(Config::Monster::MaxOverhealth * GetStatDict()->Get(Stat::Health));
    const int healingCap = static_cast<int>(Config::Monster::MaxHealing * GetStatDict()->Get(Stat::Health));
    const int headroom = healingCap - healingDone;

    if (headroom <= 0 || amount <= 0) {
        TryLog(Name + "'s healing is fully exhausted.", LType::Major);
        return;
    }
    if (amount > headroom) {
        TryLog(Name + " has " + std::to_string(headroom) + " HP of healing headroom left.", LType::Nerdy);
    }

    amount = std::clamp(amount, 0, headroom);
    HealImpl(amount);
    currentHealth = std::min(currentHealth, maxHealth);
    healingDone += amount;
}

bool Monster::ReceiveAttack(Monster *from) {
    if (!CheckIsAlive()) return false;
    return ReceiveAttackImpl(from);
}


void Monster::AttackImpl(Monster *target) {
    BaseReceiveAttack(target);
}

void Monster::BaseAttack(Monster *target) {
    TryLog(Name + " swings at " + target->Name + ".", LType::Major);
    target->ReceiveAttack(this);
}

void Monster::TakeDamageImpl(const int amount) {
    currentHealth -= amount;
    TryLog(Name + " takes " + std::to_string(amount) + " damage. (" +
           std::to_string(std::max(0, currentHealth)) + " HP remaining)", LType::Minor);
}

void Monster::HealImpl(const int amount) {
    currentHealth += amount;
    TryLog(Name + " recovers " + std::to_string(amount) + " HP.", LType::Minor);
}

bool Monster::ReceiveAttackImpl(Monster *from) {
    return BaseReceiveAttack(from);
}

bool Monster::BaseReceiveAttack(Monster *from) {
    const int hitChance = static_cast<int>(
        CalculateHitChance(from->GetStatDict()->Get(Stat::Offense), GetStatDict()->Get(Stat::Defence)));
    TryLog(Name + " has a " + std::to_string(hitChance) + "% chance to dodge.", LType::Nerdy);
    if (hitChance > RandomPCT()) {
        TryLog(Name + " sidesteps the blow!", LType::Major);
        return false;
    }
    TryLog(Name + " is struck!", LType::Major);
    TakeDamage(from->GetStatDict()->Get(Stat::Damage));
    return true;
}

void Monster::TryLog(const std::string &message, const LType type) const {
    if (LogPtr == nullptr) return;
    LogPtr->Append(message, type);
}
