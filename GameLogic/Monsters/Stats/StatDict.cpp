#include "StatDict.h"

nlohmann::json StatDict::ToJson() const {
    nlohmann::json data;
    for (int i = 0; i < static_cast<int>(Stat::COUNT); i++)
        data[StatInfos[i].AsString] = values[i];
    return data;
}

StatDict StatDict::FromJson(nlohmann::json data) {
    std::array<int, static_cast<int>(Stat::COUNT)> initValues{};
    for (int i = 0; i < static_cast<int>(Stat::COUNT); i++)
        initValues[i] = data.contains(StatInfos[i].AsString)
                            ? data[StatInfos[i].AsString].get<int>()
                            : StatInfos[i].DefaultValue;
    return StatDict(initValues);
}

StatDict::StatDict(const std::array<int, static_cast<int>(Stat::COUNT)> &initValues) : values(initValues) {
}

void StatDict::ReceiveEXP(const int amount) {
    values[static_cast<int>(Stat::Exp)] += amount;
    while (Get(Stat::Exp) >= Get(Stat::Level) * 2) {
        values[static_cast<int>(Stat::Exp)] -= Get(Stat::Level) * 2;
        values[static_cast<int>(Stat::Level)]++;
        values[static_cast<int>(Stat::SkillPoints)]++;
    }
}

int StatDict::Get(const Stat toGet) const {
    return values.at(static_cast<int>(toGet));
}

bool StatDict::TryLevel(const Stat toLevel, std::string *err) {
    const StatInfo &info = StatInfos[static_cast<int>(toLevel)];
    if (!info.Levelable) {
        SET_ERR("Stat canNOT be leveled.");
        return false;
    }

    if (Get(Stat::SkillPoints) <= 0) {
        SET_ERR("No skill-points available.");
        return false;
    }
    values[static_cast<int>(toLevel)] += info.LevelUpAmount;
    values[static_cast<int>(Stat::SkillPoints)]--;
    return true;
}
