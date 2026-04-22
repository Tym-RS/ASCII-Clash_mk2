#ifndef ASCII_CLASH_SAVEMANAGER_H
#define ASCII_CLASH_SAVEMANAGER_H
#include <sqlite3.h>
#include <string>
#include <nlohmann/json.hpp>

#include "TableDefs.h"


namespace Database {
    class SaveManager {
    public:
        explicit SaveManager(const std::string &dbName);

        std::vector<nlohmann::json> LoadWhere(
            const std::string &table,
            const std::vector<std::pair<std::string, std::string> > &conditions,
            const std::string &orderBy = "",
            int limit = -1,
            std::string *err = nullptr
        ) const;

        void SaveTo(Table t, nlohmann::json j, std::string *err = nullptr) const;

        ~SaveManager();

    private:
        const std::string DBPaths = "Database/DB_Files/";
        sqlite3 *db{};
    };
}

#endif
