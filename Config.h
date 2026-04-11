#ifndef ASCII_CLASH_CONFIG_H
#define ASCII_CLASH_CONFIG_H


#define MONSTER_STATS \
X(Damage,  "How much damage each hit from this monster deals.", 1, 1) \
X(Offense, "How likely it is, for an attack from this monster to land.", 1, 1) \
X(Defense, "How likely it is, that this monster dodges an incomming attack.", 1, 1) \
X(Special, "Different for each monster. Refer to its description.", 1, 1) \
X(Health,  "The maximum HP this monster can have.", 10, 5) \
X(Exp,     "How much EXP this monster has. (It needs [2*Level] to level up.)", 0, 0) \
X(Level,   "The current level of this monster. (This times two is how much EXP it needs to gain a new level.)", 1, 0) \
X(SkillPoints, "Each skill-point can level one stat of it's monster. A new point is gained on level-up.", 3, 0)


#define MONSTER_TYPES \
X(Human,  "Average joe. Works from 9 to 5.") \
X(Orc,    "A big GREEN savage.") \
X(Methog, "Hedgehog, but with Metal.") \
X(Ratmen, "A stealth-sneak rat.")


namespace Config {
    namespace Monsters {
        // The base chance to hit. Defense will subtract from this, attack will add.
        inline constexpr float BaseHitChance = .5;
        // Attack will ALWAYS have this chance to land.
        inline constexpr float MinHitChance = .1;
        // These will be used as X in the hit/defend chance calculation :   chance = X / (X + Stat[attack or defense])
        inline constexpr int AttackStatCounter = 25, DefenseStatCounter = 20;
    }

    namespace Players {
        inline constexpr int TeamSize = 5;
    }

    namespace Server {
        inline constexpr long SessionTimeout = 60;
    }
}

#endif
