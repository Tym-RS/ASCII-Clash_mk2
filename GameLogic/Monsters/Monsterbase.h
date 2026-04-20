#ifndef ASCII_CLASH_MONSTERBASE_H
#define ASCII_CLASH_MONSTERBASE_H

#include <string>
#include "StatDict.h"
#include "../Fighting/NestedLogger.h"
#include "Descriptions.h"

class Monster {
public:
    const std::string Name;
    const int ID;
    const MonsterType Type;
    const bool IsHealer = false;

    [[nodiscard]] StatDict *GetStatDict() { return &stats; }

    NestedLogger *LogPtr = nullptr;

    explicit Monster(std::string name, int ID, MonsterType type);

    explicit Monster(std::string name, int ID, MonsterType type, const StatDict &stats);

    [[nodiscard]] bool CheckIsAlive() const { return currentHealth > 0; }

    [[nodiscard]] int GetCurrentHealth() const { return currentHealth; }

    void Attack(Monster *target) {
        if (!CheckIsAlive() || !target->CheckIsAlive()) return;
        AttackImpl(target);
    }

    void TakeDamage(const int amount) {
        if (!CheckIsAlive()) return;
        TakeDamageImpl(amount);
        if (currentHealth <= 0) {
            currentHealth = 0;
            OnDeath();
        }
    }

    void Heal(const int amount) {
        if (!CheckIsAlive()) return;
        HealImpl(amount);
    }

    bool ReceiveAttack(Monster *from) {
        if (!CheckIsAlive()) return false;
        return ReceiveAttackImpl(from);
    }

    virtual void OnTurnStart() {
    }

    virtual void OnDeath() {
    }

    virtual ~Monster() = default;

protected:
    int currentHealth{};

    void TryLog(const std::string &message, LType type) const;

    virtual void AttackImpl(Monster *target);

    virtual void TakeDamageImpl(int amount);

    virtual void HealImpl(int amount);

    virtual bool ReceiveAttackImpl(Monster *from);

private:
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
