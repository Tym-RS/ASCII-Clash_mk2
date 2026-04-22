#ifndef ASCII_CLASH_TABLEDEFS_H
#define ASCII_CLASH_TABLEDEFS_H
#include <array>
#include <map>
#include <string>

#define LOOKUP_TABLE(table) Database::TableStringMap.at(Database::Table::table)
#define COL(table, col)     Database::table::ColStringMap.at(Database::table::Col::col)

namespace Database {
#define TABLES          X(Teams) X(Players) X(Fights) X(FightTeams) //X(Monsters)

    //#define Monsters_COLS   Y(ID, "INTEGER PRIMARY KEY") Y(Name, "TEXT NOT NULL") Y(Stats, "TEXT NOT NULL")
#define Teams_COLS      Y(ID, "INTEGER PRIMARY KEY") Y(Name, "TEXT NOT NULL UNIQUE") Y(MonsterIDs, "TEXT")
#define Players_COLS    Y(ID, "INTEGER PRIMARY KEY") Y(Username, "TEXT UNIQUE NOT NULL") Y(Password, "INTEGER NOT NULL") Y(Score, "INTEGER DEFAULT 0") Y(TeamIDs, "TEXT")
#define Fights_COLS     Y(ID, "INTEGER PRIMARY KEY") Y(WinningTeamID, "INTEGER") Y(TimeCreated, "TEXT NOT NULL") Y(Log, "TEXT")
#define FightTeams_COLS Y(FightID,"INTEGER PRIMARY KEY") Y(TeamID,"INTEGER NOT NULL")

    enum class Table {
#define X(table) table,
        TABLES
        COUNT
#undef X
    };

    const inline std::map<Table, std::string> TableStringMap = {
#define X(table) {Table::table, #table},
        TABLES
#undef X
    };

    //Col enums
#define Y(col, ...) col,
#define X(table) namespace table {enum class Col { table##_COLS };}
    TABLES
#undef X
#undef Y

    //ColStringMaps
#define Y(col, ...) {Col::col, #col},
#define X(table) namespace table { const inline std::map<Col, std::string> ColStringMap = { table##_COLS }; }
    TABLES
#undef X
#undef Y

    // Table Creating strings
    inline const std::array CreateTableStrings = {
#define Y(col, type) ", " #col " " type
#define X(table) std::string("CREATE TABLE IF NOT EXISTS " #table " (") + (table##_COLS + 2) + "),",
        TABLES
#undef X
#undef Y
    };
}
#endif
