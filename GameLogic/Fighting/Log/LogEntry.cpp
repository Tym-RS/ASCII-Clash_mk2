#include "NestedLogger.h"

NestedLogger::LogEntry::LogEntry(std::string header, LType importance)
    : Header(std::move(header)), Importance(importance) {}

void NestedLogger::LogEntry::Append(const LogEntry &toAdd) {
    logs.emplace_back(toAdd);
}

std::string NestedLogger::LogEntry::AsStr() const {
    std::string whole = Header;
    for (const auto &l: logs)
        whole.append(l.AsStr()).append("\n");
    return whole;
}

nlohmann::json NestedLogger::LogEntry::AsJson() const {
    nlohmann::json j;
    j["header"] = Header;
    j["importance"] = StringLogTypeMap.at(Importance);
    j["details"] = nlohmann::json::array();
    for (const auto &child: logs) j["details"].push_back(child.AsJson());
    return j;
}