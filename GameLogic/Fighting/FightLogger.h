#ifndef ASCII_CLASH_LOGGER_H
#define ASCII_CLASH_LOGGER_H

#include <string>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>
#include "Config.h"

enum class LType {
#define X(type) type,
    LOG_TYPES
#undef X
};

const inline std::map<std::string, LType> LogTypeStringMap{
#define X(type) {#type, LType::type},
    LOG_TYPES
#undef X
};

const inline std::map<LType, std::string> StringLogTypeMap{
#define X(type) {LType::type, #type},
    LOG_TYPES
#undef X
};


struct LogEntry {
    explicit LogEntry(std::string Header, const LType importance = LType::Minor)
        : Header(std::move(Header)), Importance(importance) {
    }

    const std::string Header;
    const LType Importance;

    void Append(const LogEntry &toAdd) {
        logs.emplace_back(toAdd);
    }

    [[nodiscard]] std::string AsStr() const {
        std::string whole = Header;
        for (const auto &l: logs) {
            whole.append(l.AsStr());
            whole.append("\n");
        }
        return whole;
    }

    [[nodiscard]] nlohmann::json AsJson() const {
        nlohmann::json j;
        j["header"] = Header;
        j["importance"] = StringLogTypeMap.at(Importance);
        j["details"] = nlohmann::json::array();
        for (const auto &child: logs) j["details"].push_back(child.AsJson());
        return j;
    }

private:
    std::vector<LogEntry> logs = {};
};

class FightLogger {
public:
    FightLogger() : StartTime(time(nullptr)) {
        // Seed the log with a root entry so Append() can never be called on an
        // empty vector. Without this, log.at(log.size() - 1) wraps to a huge
        // index on the first call and throws std::out_of_range.
        log.emplace_back("Fight Start", LType::Minor);
    }

    // Takes message by value for the same reason as LogEntry's ctor:
    // allows temporaries/literals and avoids silently moving from caller's variable.
    void Append(std::string message, const LType type = LType::Minor) {
        log.back().Append(LogEntry(std::move(message), type)); // back() is safe — log is always non-empty after ctor
    }

    void Next(std::string message, const LType type = LType::Minor) {
        log.emplace_back(std::move(message), type); // starts a new top-level turn entry
    }

    [[nodiscard]] std::string AsStr() const {
        std::string whole;
        for (const auto &entry: log) {
            whole.append(entry.AsStr());
            whole.append("\n");
        }
        return whole;
    }

    [[nodiscard]] nlohmann::json AsJson() const {
        nlohmann::json j;

        j["start_time"] = StartTime; // unix timestamp — useful for replays / sorting
        j["turns"] = nlohmann::json::array();

        for (const auto &entry: log)
            j["turns"].push_back(entry.AsJson()); // each turn is a full LogEntry tree

        return j;
    }

    const long StartTime;

private:
    std::vector<LogEntry> log;
};

#endif
