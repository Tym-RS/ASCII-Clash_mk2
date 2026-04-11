#ifndef ASCII_CLASH_MONSTERS_H
#define ASCII_CLASH_MONSTERS_H

#include <string>
#include "StatDict.h"
#include "../Fighting/Logger.h"
#include "Descriptions.h"


class Monster {
public:
    const std::string Name;
    const int ID;
    const MonsterType Type;

    [[nodiscard]] StatDict &GetStatDict() { return stats; }

    Logger *LogPtr = nullptr;

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

    void TryLog(const std::string &message, LogType type) const;

private:
    StatDict stats = StatDict();
};


// ---------------------------------------------------------------


class Human final : public Monster {
public:
    explicit Human(const std::string &name, const int id) : Monster(name, id, MonsterType::Human) {
    }

    bool ReceiveAttack(Monster *from) override;
};

class Orc final : public Monster {
public:
    explicit Orc(const std::string &name, const int id) : Monster(name, id, MonsterType::Orc) {
    }

    void Attack(Monster *target) override;
};

class Methog final : public Monster {
public:
    explicit Methog(const std::string &name, const int id) : Monster(name, id, MonsterType::Methog) {
    }

    bool ReceiveAttack(Monster *from) override;
};


#endif
