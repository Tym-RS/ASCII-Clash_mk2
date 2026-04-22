#ifndef ASCII_CLASH_MONSTERBASE_H
#define ASCII_CLASH_MONSTERBASE_H

#include <string>
#include "Stats/StatDict.h"
#include "../Fighting/Log/NestedLogger.h"
#include "Descriptions.h"
#include "Database/JsonSavable.h"

class Monster : public JsonSavable<Monster> {
public:
    const int ID;
    const std::string Name;
    const MonsterType Type;
    virtual bool IsHealer() { return false; }

    static Monster *FromJson(const nlohmann::json &j);

    nlohmann::json ToJson() override;

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

    virtual void OnTurnStart() {
    }

    virtual void OnDeath() {
    }

    ~Monster() override = default;

protected:
    void TryLog(const std::string &message, LType type) const;

    virtual void ResetImpl() {
    }

    virtual void AttackImpl(Monster *target);

    virtual void TakeDamageImpl(int amount);

    virtual void HealImpl(int amount);


    virtual bool ReceiveAttackImpl(Monster *from);

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

#endif
