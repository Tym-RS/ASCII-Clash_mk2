#ifndef ASCII_CLASH_STATDICT_H
#define ASCII_CLASH_STATDICT_H
#include "Database/JsonSavable.h"
#include "Stats.h"
#include <string>
#include <array>

#include "ErrHelper.h"

class StatDict final : public JsonSavable {
public:
    StatDict() = default;

    static StatDict FromJson(nlohmann::json j);

    [[nodiscard]] nlohmann::json ToJson() const override;

    void ReceiveEXP(int amount);

    [[nodiscard]] int Get(Stat stat) const;

    bool TryLevel(Stat toLevel, ERR_PARAM);

private:
    explicit StatDict(const std::array<int, static_cast<int>(Stat::COUNT)> &initValues);

    std::array<int, static_cast<int>(Stat::COUNT)> values{
#define X(stat, desc, dflt, lvlup) dflt,
        MONSTER_STATS
#undef X
    };
};
#endif
