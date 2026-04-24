#ifndef SERVERHELPERS_H
#define SERVERHELPERS_H
#include "Imports/httplib.h"


inline std::optional<std::string> GetCookie(const std::string &key, const httplib::Request &req) {
    const auto it = req.headers.find("Cookie");
    if (it == req.headers.end()) return std::nullopt;
    std::istringstream ss(it->second);
    std::string token;
    while (std::getline(ss, token, ';')) {
        const size_t start = token.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        token = token.substr(start);
        const std::size_t eq = token.find('=');
        if (eq == std::string::npos) continue;
        if (token.substr(0, eq) == key) return token.substr(eq + 1);
    }
    return std::nullopt;
}


#endif // SERVERHELPERS_H
