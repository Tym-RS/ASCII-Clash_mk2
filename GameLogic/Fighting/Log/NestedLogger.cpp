#include "NestedLogger.h"
#include <ctime>

const std::map<std::string, LType> LogTypeStringMap{
#define X(type) {#type, LType::type},
    LOG_TYPES
#undef X
};

const std::map<LType, std::string> StringLogTypeMap{
#define X(type) {LType::type, #type},
    LOG_TYPES
#undef X
};

NestedLogger::NestedLogger() : startTime(time(nullptr)) {
}

nlohmann::json NestedLogger::ToJson() const {
    nlohmann::json j;
    j["start_time"] = startTime;
    j["turns"] = nlohmann::json::array();
    for (const auto &entry: log) j["turns"].push_back(entry.ToJson());
    return j;
}

NestedLogger NestedLogger::FromJson(nlohmann::json j) {
    auto logger = NestedLogger();
    if (j.contains("start_time")) logger.startTime = j["start_time"].get<std::time_t>();
    if (j.contains("turns"))
        for (const auto &entryJson: j["turns"])
            logger.log.emplace_back(LogEntry::FromJson(entryJson));
    return logger;
}

void NestedLogger::Append(std::string message, const LType type) {
    if (!log.empty()) log.back().Append(LogEntry(std::move(message), type));
    else Next(std::move(message), type);
}

void NestedLogger::Next(std::string message, LType type) {
    log.emplace_back(std::move(message), type);
}

std::string NestedLogger::ToStr() const {
    std::string whole;
    for (const auto &entry: log)
        whole.append(entry.ToStr()).append("\n");
    return whole;
}
