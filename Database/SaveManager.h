#pragma once

#include <sqlite3.h>
#include <string>
#include <nlohmann/json.hpp>
#include "ErrHelper.h"


namespace Database {
    class SaveManager {
    public:
        explicit SaveManager(const std::string &dbName);

        std::vector<nlohmann::json> LoadWhere(
            const std::string &table,
            const std::vector<std::pair<std::string, std::string> > &conditions = {},
            const std::string &orderBy = "",
            int limit = -1,
            ERR_PARAM) const;

        bool TrySaveTo(const std::string &table, nlohmann::json j, ERR_PARAM) const;

        [[nodiscard]] int NextID(const std::string &table) const;

#ifndef NDEBUG
        void DebugDump() const;
#endif


        ~SaveManager();

    private:
        void EnsureColumns(const std::string &table, const nlohmann::json &j) const;

        const std::string DBPaths = "Database/DB_Files/";
        sqlite3 *db{};
    };
}
