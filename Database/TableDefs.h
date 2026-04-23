#ifndef ASCII_CLASH_TABLEDEFS_H
#define ASCII_CLASH_TABLEDEFS_H
#include <array>
#include <map>
#include <string>

#define LOOKUP_TABLE(table) Database::TableStringMap.at(Database::Table::table)
#define COL(table, col)     Database::table::ColStringMap.at(Database::table::Col::col)

namespace Database {
#define TABLES          X(Teams) X(Players) X(Fights) X(FightTeams) //X(Monsters)

#define Players_COLS    Y(ID, "INTEGER PRIMARY KEY") Y(username, "TEXT UNIQUE NOT NULL") Y(password, "INTEGER NOT NULL") Y(score, "INTEGER DEFAULT 0") Y(team_IDs, "TEXT")
#define Teams_COLS      Y(ID, "INTEGER PRIMARY KEY") Y(name, "TEXT NOT NULL UNIQUE") Y(monsters, "TEXT")
#define Fights_COLS     Y(ID, "INTEGER PRIMARY KEY") Y(is_ongoing, "BOOLEAN DEFAULT TRUE") Y(winner, "TEXT") \
    Y(active_team, "TEXT") Y(turn, "INTEGER DEFAULT 0 NOT NULL") Y(log, "TEXT") Y(participants, "TEXT NOT NULL")
#define FightTeams_COLS Y(fight_ID,"INTEGER PRIMARY KEY") Y(team_ID,"INTEGER NOT NULL")


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
