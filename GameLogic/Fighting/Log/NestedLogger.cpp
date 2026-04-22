#include "NestedLogger.h"
#include <ctime>

const std::map<std::string, LType> LogTypeStringMap {
#define X(type) {#type, LType::type},
    LOG_TYPES
#undef X
};

const std::map<LType, std::string> StringLogTypeMap {
#define X(type) {LType::type, #type},
    LOG_TYPES
#undef X
};

NestedLogger::NestedLogger() : StartTime(time(nullptr)) {
    log.emplace_back("Log Created", LType::Nerdy);
}

void NestedLogger::Append(std::string message, LType type) {
    log.back().Append(LogEntry(std::move(message), type));
}

void NestedLogger::Next(std::string message, LType type) {
    log.emplace_back(std::move(message), type);
}

std::string NestedLogger::AsStr() const {
    std::string whole;
    for (const auto &entry: log)
        whole.append(entry.AsStr()).append("\n");
    return whole;
}

nlohmann::json NestedLogger::AsJson() const {
    nlohmann::json j;
    j["start_time"] = StartTime;
    j["turns"] = nlohmann::json::array();
    for (const auto &entry: log) j["turns"].push_back(entry.AsJson());
    return j;
}