#ifndef ASCII_CLASH_TEAM_H
#define ASCII_CLASH_TEAM_H
#include "Config.h"
#include "Fighting/Fight.h"
#include "Monsters/Monsterbase.h"


class Team final : public JsonSavable {
public:
    const std::string Name;
    const int ID;

    bool AutoFight = false;

    explicit Team(std::string name, int id);

    static Team FromJson(const nlohmann::json &j);

    [[nodiscard]] nlohmann::json ToJson() const override;

    [[nodiscard]] bool IsInFight() const { return isInFight; }

    void EnterFight(NestedLogger *l);

    void ExitFight(int expGain = 0);

    [[nodiscard]]

    const std::array<std::unique_ptr<Monster>, Config::Team::Size> &Monsters() const { return monsters; }

    [[nodiscard]] std::array<std::unique_ptr<Monster>, Config::Team::Size> &Monsters() { return monsters; }

private:
    bool isInFight = false;
    std::array<std::unique_ptr<Monster>, Config::Team::Size> monsters{};
};


#endif
