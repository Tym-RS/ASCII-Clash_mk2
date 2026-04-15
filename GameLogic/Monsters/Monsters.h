#ifndef ASCII_CLASH_MONSTERS_H
#define ASCII_CLASH_MONSTERS_H

#include <string>
#include "StatDict.h"
#include "../Fighting/FightLogger.h"
#include "Descriptions.h"

class Monster {
public:
    const std::string Name;
    const int ID;
    const MonsterType Type;

    [[nodiscard]] StatDict *GetStatDict() { return &stats; }

    FightLogger *LogPtr = nullptr;

    explicit Monster(std::string name, int ID, MonsterType type);

    explicit Monster(std::string name, int ID, MonsterType type, const StatDict &stats);

    [[nodiscard]] bool IsAlive() const;

    void virtual OnTurnStart() {
    }

    void virtual Attack(Monster *target);

    void virtual TakeDamage(int amount);

    void virtual Heal(int amount);

    bool virtual ReceiveAttack(Monster *from);

    virtual ~Monster() = default;

protected:
    int currentHealth{};

    void TryLog(const std::string &message, LType type) const;

private:
    StatDict stats = StatDict();
};


// ---------------------------------------------------------------
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


class Human final : public TypedMonster<MonsterType::Human> {
public:
    using TypedMonster::TypedMonster;

    bool ReceiveAttack(Monster *from) override;
};

class Orc final : public TypedMonster<MonsterType::Orc> {
public:
    using TypedMonster::TypedMonster;

    void Attack(Monster *target) override;
};

class Methog final : public TypedMonster<MonsterType::Methog> {
public:
    using TypedMonster::TypedMonster;

    bool ReceiveAttack(Monster *from) override;
};

class Ratkin final : public TypedMonster<MonsterType::Ratkin> {
public:
    using TypedMonster::TypedMonster;
};


#include "Config.h"


inline Monster *CreateTypedMonster(const std::string &name, const int id, const MonsterType type,
                                   const StatDict *stats = nullptr) {
    switch (type) {
#define X(type, ...) case MonsterType::type: return stats? new type(name, id, *stats) : new type(name, id);
        MONSTER_TYPES
#undef X
        default: return stats
                            ? new Monster(name, id, MonsterType::None, *stats)
                            : new Monster(name, id, MonsterType::None);
    }
}


#endif
