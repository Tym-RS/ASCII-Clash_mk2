#ifndef ASCII_CLASH_LOGGER_H
#define ASCII_CLASH_LOGGER_H
#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Config.h"

enum class LType {
#define X(type) type,
    LOG_TYPES
#undef X
};

extern const std::map<std::string, LType> LogTypeStringMap;
extern const std::map<LType, std::string> StringLogTypeMap;

class NestedLogger {
public:
    NestedLogger();
    void Append(std::string message, LType type = LType::Minor);
    void Next(std::string message, LType type = LType::Minor);
    [[nodiscard]] std::string AsStr() const;
    [[nodiscard]] nlohmann::json AsJson() const;

    const long StartTime;

private:
    struct LogEntry {
        explicit LogEntry(std::string header, LType importance = LType::Minor);
        void Append(const LogEntry &toAdd);
        [[nodiscard]] std::string AsStr() const;
        [[nodiscard]] nlohmann::json AsJson() const;

        const std::string Header;
        const LType Importance;
    private:
        std::vector<LogEntry> logs;
    };

    std::vector<LogEntry> log;
};

#endif