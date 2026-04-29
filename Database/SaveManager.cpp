#include <iostream>
#include <string>

#include "SaveManager.h"
#include "TableDefs.h"

namespace Database {
    SaveManager::SaveManager(const std::string &dbName) {
        const std::string path = DBPaths + dbName + ".db";
        sqlite3_open(path.c_str(), &db);
        for (const auto &cmd: CreateTableStrings) {
            char *err = nullptr;
            sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, &err);
            if (!err) continue;
            const std::string msg = err;
            sqlite3_free(err);
            throw std::runtime_error(msg);
        }
    }

    std::vector<nlohmann::json> SaveManager::LoadWhere(const std::string &table,
                                                       const std::vector<std::pair<std::string, std::string> > &
                                                       conditions,
                                                       const std::string &orderBy,
                                                       const int limit,
                                                       std::string *err
    ) const {
        std::string cmd = "SELECT * FROM " + table;
        if (!conditions.empty()) {
            cmd += " WHERE ";
            for (int i = 0; i < static_cast<int>(conditions.size()); i++) {
                if (i > 0) cmd += " AND ";
                cmd += conditions[i].first + " = ?";
            }
        }
        if (!orderBy.empty()) cmd += " ORDER BY " + orderBy;
        if (limit > 0) cmd += " LIMIT " + std::to_string(limit);
        cmd += ";";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            if (err) *err = sqlite3_errmsg(db);
            return {};
        }
        for (int i = 0; i < static_cast<int>(conditions.size()); i++)
            sqlite3_bind_text(stmt, i + 1, conditions[i].second.c_str(), -1, SQLITE_STATIC);

        std::vector<nlohmann::json> result;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            nlohmann::json row;
            const int colCount = sqlite3_column_count(stmt);
            for (int i = 0; i < colCount; i++) {
                const std::string colName = sqlite3_column_name(stmt, i);
                switch (sqlite3_column_type(stmt, i)) {
                    case SQLITE_INTEGER:
                        row[colName] = sqlite3_column_int(stmt, i);
                        break;
                    case SQLITE_FLOAT:
                        row[colName] = sqlite3_column_double(stmt, i);
                        break;
                    case SQLITE_TEXT: {
                        std::string text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                        try { row[colName] = nlohmann::json::parse(text); } catch (...) { row[colName] = text; }
                        break;
                    }
                    default:
                        row[colName] = nullptr;
                        break;
                }
            }
            result.push_back(row);
        }
        sqlite3_finalize(stmt);
        if (result.empty())
            SET_ERR("No rows found in " + table);
        return result;
    }

    bool SaveManager::TrySaveTo(const std::string &table, nlohmann::json j, std::string *err) const {
        std::vector<std::string> keys;
        for (auto &[key, _]: j.items()) keys.push_back(key);

        std::string colList, placeholders;
        for (int i = 0; i < static_cast<int>(keys.size()); i++) {
            if (i > 0) {
                colList += ", ";
                placeholders += ", ";
            }
            colList += keys[i];
            placeholders += "?";
        }

        const std::string cmd = "INSERT OR REPLACE INTO " + table + " (" + colList + ") VALUES (" + placeholders + ");";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);

        for (int i = 0; i < static_cast<int>(keys.size()); i++) {
            auto &v = j[keys[i]];
            if (v.is_number_integer()) sqlite3_bind_int(stmt, i + 1, v.get<int>());
            else if (v.is_number_float()) sqlite3_bind_double(stmt, i + 1, v.get<double>());
            else if (v.is_string()) {
                auto s = v.get<std::string>();
                sqlite3_bind_text(stmt, i + 1, s.c_str(), -1, SQLITE_TRANSIENT);
            } else if (v.is_null()) sqlite3_bind_null(stmt, i + 1);
            else {
                auto s = v.dump();
                sqlite3_bind_text(stmt, i + 1, s.c_str(), -1, SQLITE_TRANSIENT);
            }
        }

        const int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE)
            SET_ERR(sqlite3_errmsg(db));
        return rc == SQLITE_DONE;
    }

    int SaveManager::NextID(const std::string &table) const {
        const std::string cmd = "SELECT COALESCE(MAX(id), 0) + 1 FROM " + table + ";";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return 1;
        int nextId = 1;
        if (sqlite3_step(stmt) == SQLITE_ROW) nextId = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return nextId;
    }

    SaveManager::~SaveManager() {
        sqlite3_close(db);
    }


#ifndef NDEBUG


    void SaveManager::DebugDump() const {
        const std::string divider(60, '=');
        const std::string subDivider(60, '-');

        // Collect all user table names from sqlite_master
        std::vector<std::string> tables;
        {
            sqlite3_stmt *stmt;
            const std::string cmd = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
            if (sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW)
                    tables.emplace_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
                sqlite3_finalize(stmt);
            }
        }

        std::cout << divider << "\n";
        std::cout << "  SAVE MANAGER DEBUG DUMP  (" << tables.size() << " tables)\n";
        std::cout << divider << "\n\n";

        for (const auto &table: tables) {
            // Row count
            int rowCount = 0;
            {
                sqlite3_stmt *stmt;
                const std::string cmd = "SELECT COUNT(*) FROM " + table + ";";
                if (sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    if (sqlite3_step(stmt) == SQLITE_ROW) rowCount = sqlite3_column_int(stmt, 0);
                    sqlite3_finalize(stmt);
                }
            }

            std::cout << "[ TABLE: " << table << " ]  (" << rowCount << " rows)\n";
            std::cout << subDivider << "\n";

            if (rowCount == 0) {
                std::cout << "  (empty)\n\n";
                continue;
            }

            sqlite3_stmt *stmt;
            const std::string cmd = "SELECT * FROM " + table + ";";
            if (sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                std::cout << "  [ERROR: " << sqlite3_errmsg(db) << "]\n\n";
                continue;
            }

            const int colCount = sqlite3_column_count(stmt);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                for (int i = 0; i < colCount; i++) {
                    const std::string colName = sqlite3_column_name(stmt, i);
                    std::string value;
                    switch (sqlite3_column_type(stmt, i)) {
                        case SQLITE_INTEGER:
                            value = std::to_string(sqlite3_column_int(stmt, i));
                            break;
                        case SQLITE_FLOAT:
                            value = std::to_string(sqlite3_column_double(stmt, i));
                            break;
                        case SQLITE_TEXT: {
                            const std::string text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                            // Collapse JSON objects/arrays to a placeholder
                            if (!text.empty() && (text.front() == '{' || text.front() == '['))
                                value = (text.front() == '{') ? "{...}" : "[...]";
                            else
                                value = text;
                            break;
                        }
                        case SQLITE_NULL:
                            value = "NULL";
                            break;
                        default:
                            value = "?";
                            break;
                    }
                    std::cout << "  " << colName << " : " << value << "\n";
                }
                std::cout << subDivider << "\n";
            }
            sqlite3_finalize(stmt);
            std::cout << "\n";
        }

        std::cout << divider << "\n";
        std::cout << "  END OF DUMP\n";
        std::cout << divider << "\n" << std::flush;
    }
#endif
}
