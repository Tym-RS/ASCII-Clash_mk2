#ifndef ASCII_CLASH_LOGGER_H
#define ASCII_CLASH_LOGGER_H
#include <algorithm>
#include <string>
#include <vector>

enum class LogType {
    event,
    info,
    nerd,
};

class Logger {
public:
    void Log(const std::string &message, const LogType type) {
        log.emplace_back(message, type);
    }

    [[nodiscard]] std::vector<std::string> GetFiltered(const std::vector<LogType> &selectedLogTypes) const {
        std::vector<std::string> result = {};
        for (auto &[message, type]: log)
            if (std::ranges::find(selectedLogTypes, type) != selectedLogTypes.end()) result.push_back(message);
        return result;
    }

private:
    std::vector<std::tuple<std::string, LogType> > log;
};

#endif
