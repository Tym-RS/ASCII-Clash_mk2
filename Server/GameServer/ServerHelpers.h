#ifndef SERVERHELPERS_H
#define SERVERHELPERS_H
#include "Imports/httplib.h"

#define REQUIRE_PARAMS(...)                                     \
    for (const std::string _p : {__VA_ARGS__}) {                \
        if (req.has_param(_p))  continue;                       \
        res.status = 400;                                       \
        res.set_content("Param [ " + _p + " ] was NOT found.", "text/plain");  \
        return;                                                 \
        }
#define REQUIRE_HEADERS(...)                                    \
    for (const std::string _p : {__VA_ARGS__}) {                \
        if (req.has_header(_p))  continue;                      \
        res.status = 400;                                       \
        res.set_content("Header [ " + _p + " ] was NOT found.", "text/plain");  \
        return;                                                 \
        }

#define RETURN_RES(text, code) {res.status = code; res.set_content(text, "text/plain"); return;}

inline std::string GetCookie(const std::string &cookie, REQ_PARAMS) {
    const auto it = req.headers.find("Cookie");
    if (it == req.headers.end()) return "";
    std::istringstream ss(it->second);
    std::string token;
    while (std::getline(ss, token, ';')) {
        const size_t start = token.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        token = token.substr(start);
        const std::size_t eq = token.find('=');
        if (eq == std::string::npos) continue;
        if (token.substr(0, eq) == cookie) return token.substr(eq + 1);
    }
    return "";
}

inline void SetCookie(const std::string &cookie, const std::string &val, REQ_PARAMS) {
    res.set_header("Set-Cookie", cookie + "=" + val + "; HttpOnly; Path=/; SameSite=Strict");
}

inline void DeleteCookie(const std::string &cookie, REQ_PARAMS) {
    res.set_header("Set-Cookie", cookie + "=; Max-Age=0; HttpOnly; Path=/; SameSite=Strict");
}


inline int Hash(const std::string &toHash) { return std::hash<std::string>{}(toHash); }


#endif
