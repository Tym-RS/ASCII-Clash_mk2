#pragma once

#include <string>
#include "Stats/StatDict.h"
#include "../Fighting/Log/NestedLogger.h"
#include "MonsterClasses/_MonDescriptions.h"
#include "Database/JsonSavable.h"

inline float SoftRatio(const int value, const int counter) { return static_cast<float>(value) / (counter + value); }


inline float CalculateHitChance(const int attack, const int defence) {
    const float attackBonus = SoftRatio(attack, Config::Monster::AttackStatCounter);
    const float defenseBonus = SoftRatio(defence, Config::Monster::DefenseStatCounter);
    const float chance = Config::Monster::BaseHitChance + attackBonus - defenseBonus;
    return chance > Config::Monster::MinHitChance ? chance : Config::Monster::MinHitChance;
}

inline float RandomPCT() { return static_cast<float>(rand()) / RAND_MAX; }


class Monster : public JsonSavable {
public:
    const int ID;
    const std::string Name;
    const MonsterType Type;
    [[nodiscard]] virtual bool IsHealer() const { return false; }

    static std::unique_ptr<Monster> FromJson(const nlohmann::json &data);

    [[nodiscard]] nlohmann::json ToJson() const override;


    [[nodiscard]] StatDict *GetStatDict() { return &stats; }

    NestedLogger *LogPtr = nullptr;

    explicit Monster(std::string name, int id, MonsterType type);

    explicit Monster(std::string name, int id, MonsterType type, StatDict stats);

    [[nodiscard]] bool CheckIsAlive() const { return currentHealth > 0; }

    [[nodiscard]] int GetCurrentHealth() const { return currentHealth; }

    void Reset();

    void Attack(Monster *target);

    void TakeDamage(int amount);

    void Heal(int amount);

    bool ReceiveAttack(Monster *from);

    virtual void OnTurnStart() { return; }

    virtual void OnDeath() { return; }

    ~Monster() override = default;

protected:
    void TryLog(const std::string &message, LType type) const;


    [[nodiscard]] virtual nlohmann::json ToJsonImpl() const { return {}; }

    virtual void FromJsonImpl(const nlohmann::json &j) { return; }

    virtual void ResetImpl() { return; }

    virtual void AttackImpl(Monster *target);

    void BaseAttack(Monster *target);

    virtual void TakeDamageImpl(int amount);

    virtual void HealImpl(int amount);


    virtual bool ReceiveAttackImpl(Monster *from);

    bool BaseReceiveAttack(Monster *from);

private:
    int currentHealth{}, healingDone = 0;
    StatDict stats = StatDict();
};


template<MonsterType T>
class TypedMonster : public Monster {
public:
    explicit TypedMonster(const std::string &name, const int id)
        : Monster(name, id, T) {
    }

    explicit TypedMonster(const std::string &name, const int id, const StatDict &stats)
        : Monster(name, id, T, stats) {
    }
};
