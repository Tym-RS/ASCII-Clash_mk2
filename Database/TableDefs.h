#ifndef ASCII_CLASH_TABLEDEFS_H
#define ASCII_CLASH_TABLEDEFS_H
#include <array>
#include <map>
#include <string>

#define TABLE(table) Database::TableStringMap.at(Database::Table::table)
#define COL(table, col)     Database::table::ColStringMap.at(Database::table::Col::col)

namespace Database {
#define TABLES          X(Teams) X(Players) X(Fights)
#define Players_COLS    Y(ID, "INTEGER PRIMARY KEY") Y(username, "TEXT UNIQUE NOT NULL") Y(password, "INTEGER NOT NULL") Y(score, "INTEGER DEFAULT 0") Y(teams, "TEXT")
#define Teams_COLS      Y(ID, "INTEGER PRIMARY KEY") Y(name, "TEXT NOT NULL UNIQUE") Y(monsters, "TEXT") Y(fight_id, "INTEGER") Y(auto_fight, "BOOLEAN DEFAULT FALSE NOT NULL")
#define Fights_COLS     Y(ID, "INTEGER PRIMARY KEY") Y(is_ongoing, "BOOLEAN DEFAULT TRUE NOT NULL") Y(winner, "TEXT") \
                        Y(active_team_name, "TEXT") Y(turn, "INTEGER DEFAULT 0 NOT NULL") Y(log, "TEXT") Y(participants, "TEXT NOT NULL")


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
#define X(table) std::string("CREATE TABLE IF NOT EXISTS " #table " (") + (table##_COLS + 2) + ");",
        TABLES
#undef X
#undef Y
    };
}
#endif
