#include "NestedLogger.h"

NestedLogger::LogEntry::LogEntry(std::string header, LType importance)
    : header(std::move(header)), importance(importance) {
}

void NestedLogger::LogEntry::Append(const LogEntry &toAdd) {
    logs.emplace_back(toAdd);
}

NestedLogger::LogEntry NestedLogger::LogEntry::FromJson(nlohmann::json j) {
    const std::string header = j.at("header").get<std::string>();
    const std::string importanceStr = j.at("importance").get<std::string>();
    auto entry = LogEntry(header, LogTypeStringMap.at(importanceStr));
    if (j.contains("details"))
        for (const auto &childJson: j["details"])
            entry.logs.emplace_back(FromJson(childJson));
    return entry;
}

std::string NestedLogger::LogEntry::ToStr() const {
    std::string whole = header;
    for (const auto &l: logs)
        whole.append(l.ToStr()).append("\n");
    return whole;
}

nlohmann::json NestedLogger::LogEntry::ToJson() const {
    nlohmann::json j;
    j["header"] = header;
    j["importance"] = StringLogTypeMap.at(importance);
    j["details"] = nlohmann::json::array();
    for (const auto &child: logs) j["details"].push_back(child.ToJson());
    return j;
}
