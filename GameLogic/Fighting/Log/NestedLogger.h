#ifndef ASCII_CLASH_LOGGER_H
#define ASCII_CLASH_LOGGER_H
#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Config.h"
#include "Database/JsonSavable.h"

enum class LType {
#define X(type) type,
    LOG_TYPES
#undef X
};

extern const std::map<std::string, LType> LogTypeStringMap;
extern const std::map<LType, std::string> StringLogTypeMap;

class NestedLogger final : JsonSavable {
public:
    NestedLogger();

    [[nodiscard]] long StartTime() const { return startTime; }

    static NestedLogger FromJson(nlohmann::json j);

    [[nodiscard]] nlohmann::json ToJson() const override;

    [[nodiscard]] std::string ToStr() const;

    void Append(std::string message, LType type = LType::Minor);

    void Next(std::string message, LType type = LType::Minor);

private:
    long startTime;

    struct LogEntry final : JsonSavable {
        explicit LogEntry(std::string header, LType importance = LType::Minor);

        void Append(const LogEntry &toAdd);

        static LogEntry FromJson(nlohmann::json j);

        [[nodiscard]] nlohmann::json ToJson() const override;

        [[nodiscard]] std::string ToStr() const;

    private:
        std::string header;
        LType importance;

    private:
        std::vector<LogEntry> logs;
    };

    std::vector<LogEntry> log;
};

#endif
