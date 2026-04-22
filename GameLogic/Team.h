#ifndef ASCII_CLASH_TEAM_H
#define ASCII_CLASH_TEAM_H
#include "Config.h"
#include "Monsters/Monsterbase.h"


class Team final : public JsonSavable<Team> {
public:
    const std::string Name;
    const int ID;

    bool AutoFight = false;

    explicit Team(std::string name, int id);

    static Team *FromJson(const nlohmann::json &j);

    nlohmann::json ToJson() override;

    [[nodiscard]] std::array<Monster *, Config::Team::Size> Monsters() const { return monsters; }

    ~Team() override;

private:
    std::array<Monster *, Config::Team::Size> monsters{};
};


#endif
