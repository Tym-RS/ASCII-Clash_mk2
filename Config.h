#ifndef ASCII_CLASH_CONFIG_H
#define ASCII_CLASH_CONFIG_H
#include <regex>

#define MONSTER_STATS \
X(Damage,  "How much damage each hit from this monster deals.", 1, 1) \
X(Offense, "How likely it is, for an attack from this monster to land.", 1, 1) \
X(Defence, "How likely it is, that this monster dodges an incomming attack.", 1, 1) \
X(Special, "Different for each monster. Refer to its description.", 1, 1) \
X(Health,  "The maximum HP this monster can have.", 10, 5) \
X(Exp,     "How much EXP this monster has. (It needs [2*Level] to level up.)", 0, 0) \
X(Level,   "The current level of this monster. (This times two is how much EXP it needs to gain a new level.)", 1, 0) \
X(SkillPoints, "Each skill-point can level one stat of it's monster. A new point is gained on level-up.", 3, 0)

#define MONSTER_TYPES \
X(Human,  "Average joe. Works from 9 to 5.") \
X(Orc,    "A big GREEN savage.") \
X(Methog, "Hedgehog, but with Metal.") \
X(Ratkin, "A stealth-sneak rat.")

#define LOG_TYPES \
X(Major) \
X(Minor) \
X(Nerdy)


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
        constexpr int Size = 5;
    }

    namespace Fight {
        constexpr int maxTurnCount = 150;
    }

    namespace Player {
        inline static std::regex usernameRegex(R"(^[A-Za-z0-9_()\-:;\[\]{}]{1,15}$)");
        inline static std::regex passwordRegex(R"(^(?=.{5,})(?=.*[^A-Za-z0-9]).*$)");
        constexpr int TeamAmount = 5;
    }

    namespace Server {
        inline constexpr long SessionTimeout = 9999999;
        inline constexpr int LeaderboardSize = 100;
    }
}

#endif
