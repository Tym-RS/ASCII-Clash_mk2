#include "GameServer.h"


std::optional<std::string> GameServer::GetCookie(const std::string &key, const httplib::Request &req) {
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

std::string GameServer::RandomID() {
    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist;
    std::stringstream stream;
    for (int i = 0; i < 4; ++i) stream << std::hex << dist(rd);
    return stream.str();
}
