#ifndef ASCII_CLASH_STATDICT_H
#define ASCII_CLASH_STATDICT_H
#include "Stats.h"
#include <string>
#include <array>

class StatDict {
public:
    StatDict() = default;

    explicit StatDict(const std::array<int, static_cast<int>(Stat::COUNT)> &initValues) : values(initValues) {
    }

    void ReceiveEXP(const int amount) {
        values[static_cast<int>(Stat::Exp)] += amount;
        if (Get(Stat::Exp) < Get(Stat::Level) * 2) return;

        values[static_cast<int>(Stat::Exp)] -= Get(Stat::Level) * 2;
        values[static_cast<int>(Stat::Level)]++;
        values[static_cast<int>(Stat::SkillPoints)]++;
    }

    [[nodiscard]] int Get(const Stat stat) const {
        return values.at(static_cast<int>(stat));
    }

    bool TryLevel(const Stat toLevel, std::string *err = nullptr) {
        const StatInfo &info = StatInfos[static_cast<int>(toLevel)];
        if (!info.Levelable) {
            if (err) *err = "Stat canNOT be leveled.";
            return false;
        }

        if (Get(Stat::SkillPoints) <= 0) {
            if (err) *err = "No skill-points available.";
            return false;
        }
        values[static_cast<int>(toLevel)] += info.LevelUpAmount;
        values[static_cast<int>(Stat::SkillPoints)]--;
        return true;
    }

private:
    std::array<int, static_cast<int>(Stat::COUNT)> values{
#define X(stat, desc, dflt, lvlup) dflt,
        MONSTER_STATS
#undef X
    };
};
#endif
