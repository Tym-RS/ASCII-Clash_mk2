#ifndef ASCII_CLASH_CONFIG_H
#define ASCII_CLASH_CONFIG_H
#include <regex>


namespace Config {
    namespace Monster {
        //The maximum amount of ADDITIONAL health a monster can have.
        inline constexpr float MaxOverhealth = 1.0;
        //The maximum healing a monster can receive each reset. (Calculated as MaxHealth * X, e.g. 100hp, 1 MaxHealing -> Maximum of 100hp regenerated)
        inline constexpr float MaxHealing = 1.0;
        // The base chance to hit. Defence will subtract from this, attack will add.
        inline constexpr float BaseHitChance = .5;
        // Attack will ALWAYS have this chance to land.
        inline constexpr float MinHitChance = .1;
        // These will be used as X in the hit/defend chance calculation :   chance = X / (X + Stat[attack or defence])
        inline constexpr int AttackStatCounter = 25, DefenseStatCounter = 20;
    }

    namespace Team {
        constexpr int Size = 3;
        inline static std::regex TeamNameRegex(R"(^[A-Za-z0-9_()\-:;\[\]{}]{1,15}$)");
        inline static std::regex MonsterNameRegex(R"(^[A-Za-z0-9_()\-:;\[\]{}]{1,15}$)");
    }

    namespace Fight {
        constexpr int MaxTurnCount = 100;
    }

    namespace Player {
        inline static std::regex UsernameRegex(R"(^[A-Za-z0-9_()\-:;\[\]{}]{1,15}$)");
        inline static std::regex PasswordRegex(R"(^(?=.{5,})(?=.*[^A-Za-z0-9]).*$)");
        constexpr int TeamAmount = 2;
    }

    namespace Server {
        inline const std::string MountPath = "Server/HTML";
        inline constexpr long SessionTimeoutS = 150;
    }
}

#endif
