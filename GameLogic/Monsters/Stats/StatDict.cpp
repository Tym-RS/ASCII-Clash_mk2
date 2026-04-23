#include "StatDict.h"

nlohmann::json StatDict::ToJson() const {
    nlohmann::json j;
    for (int i = 0; i < static_cast<int>(Stat::COUNT); i++)
        j[StatInfos[i].AsString] = values[i];
    return j;
}

StatDict StatDict::FromJson(nlohmann::json j) {
    std::array<int, static_cast<int>(Stat::COUNT)> initValues{};
    for (int i = 0; i < static_cast<int>(Stat::COUNT); i++)
        initValues[i] = j.contains(StatInfos[i].AsString)
                            ? j[StatInfos[i].AsString].get<int>()
                            : StatInfos[i].DefaultValue;
    return StatDict(initValues);
}

StatDict::StatDict(const std::array<int, static_cast<int>(Stat::COUNT)> &initValues) : values(initValues) {
}

void StatDict::ReceiveEXP(const int amount) {
    values[static_cast<int>(Stat::Exp)] += amount;
    if (Get(Stat::Exp) < Get(Stat::Level) * 2) return;

    values[static_cast<int>(Stat::Exp)] -= Get(Stat::Level) * 2;
    values[static_cast<int>(Stat::Level)]++;
    values[static_cast<int>(Stat::SkillPoints)]++;
}

int StatDict::Get(const Stat stat) const {
    return values.at(static_cast<int>(stat));
}

bool StatDict::TryLevel(const Stat toLevel, std::string *err) {
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
