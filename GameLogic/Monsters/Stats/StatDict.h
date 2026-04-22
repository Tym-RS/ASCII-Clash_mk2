#ifndef ASCII_CLASH_STATDICT_H
#define ASCII_CLASH_STATDICT_H
#include "Database/JsonSavable.h"
#include "Stats.h"
#include <string>
#include <array>

class StatDict final : public JsonSavable<StatDict> {
public:
    StatDict() = default;

    static StatDict *FromJson(nlohmann::json j);

    nlohmann::json ToJson() override;

    void ReceiveEXP(int amount);

    [[nodiscard]] int Get(Stat stat) const;

    bool TryLevel(Stat toLevel, std::string *err = nullptr);

private:
    explicit StatDict(const std::array<int, static_cast<int>(Stat::COUNT)> &initValues);

    std::array<int, static_cast<int>(Stat::COUNT)> values{
#define X(stat, desc, dflt, lvlup) dflt,
        MONSTER_STATS
#undef X
    };
};
#endif
