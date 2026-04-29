#ifndef ASCII_CLASH_TEAM_H
#define ASCII_CLASH_TEAM_H
#include "Config.h"
#include "Fighting/Fight.h"
#include "Monsters/MonsterBase.h"


class Team final : public JsonSavable {
public:
    const std::string Name;
    const int ID;

    bool AutoFight = false;

    explicit Team(std::string name, int id);

    static Team FromJson(const nlohmann::json &j);

    [[nodiscard]] nlohmann::json ToJson() const override;

    [[nodiscard]] bool IsInFight() const { return fightID != -1; }

    [[nodiscard]] int CurrentFightID() const { return fightID; }

    [[nodiscard]] int GetLvl() const;

    int TryGetNewMonsterID(const std::string &name, MonsterType type, ERR_PARAM);

    [[nodiscard]] Monster *TryGetMonster(int id, ERR_PARAM) const;

    bool TryDeleteMonster(int id, ERR_PARAM);

    void EnterFight(int id, NestedLogger *l);

    void ExitFight(int expGain = 0);

    [[nodiscard]] const std::array<std::unique_ptr<Monster>, Config::Team::Size> &Monsters() const { return monsters; }
    [[nodiscard]] std::array<std::unique_ptr<Monster>, Config::Team::Size> &Monsters() { return monsters; }

private:
    int fightID = -1;
    std::array<std::unique_ptr<Monster>, Config::Team::Size> monsters{};
};


#endif
