#include "GameServer.h"
#include "./ServerHelpers.h"
#include "Endpoints.h"
#include "Imports/inja.hpp"

#define SESSION_ID const auto sessionID = GetCookie("session", req, res); if(sessionID.empty()) {RETURN_RES("Login required.", 401)}

using namespace httplib;
static const std::string SESSION_COOKIE = "session";


GameServer::GameServer(SaveManager *saveManager) : db(saveManager), memory(MemoryManager(saveManager)) {
    server.set_mount_point("/", ServerMountPath);

    // Register ALL endpoints to their POST paths.
#define X(path) server.Post("/"#path, [this](REQ_PARAMS) { this->path(req, res); });
    ENDPOINTS
#undef X
    // Register GET paths to lead to the correct HTML.
    server.Get(R"(/(.*))", [](const Request &req, Response &res) {
        const std::string folder = "Server/HTML/";
        std::string path = req.matches[1];
        if (path.empty()) path = "index.html";
        std::string fullPath = folder + path;
        if (!std::filesystem::exists(fullPath)) fullPath += ".html";
        if (!std::filesystem::exists(fullPath)) RETURN_RES("Not Found.", 404);

        std::ifstream file(fullPath);
        std::stringstream ss;
        ss << file.rdbuf();
        res.set_content(ss.str(), "text/html");
    });
}


void GameServer::Leaderboard(const Request &req, Response &res) {
}

void GameServer::GameInfo(const Request &req, Response &res) {
    RETURN_RES(GetGameDescriptionsJSON().dump(), 200);
}

void GameServer::Login(const Request &req, Response &res) {
    REQUIRE_PARAMS("username", "password");
    const std::string usr = req.get_param_value("username");
    const std::string pwd = req.get_param_value("password");
    std::string err;
    const auto sessionID = memory.GetNewSessionID(usr, Hash(pwd), &err);
    if (sessionID.empty()) RETURN_RES(err, 404);
    res.status = 200;
    SetCookie(SESSION_COOKIE, sessionID, req, res);
}

void GameServer::Register(const Request &req, Response &res) {
    REQUIRE_PARAMS("username", "password");
    const std::string usr = req.get_param_value("username");
    const std::string pwd = req.get_param_value("password");
    if (!std::regex_match(usr, Config::Player::usernameRegex))
        RETURN_RES("Username must be 1–15 chars and may only contain letters, numbers, _ ( ) - : ; [ ] { }", 400);

    if (!std::regex_match(pwd, Config::Player::passwordRegex))
        RETURN_RES("Password must be at least 5 characters and contain at least one special character", 400);

    std::string err;
    if (memory.TryRegisterPlayer(usr, Hash(pwd), &err)) {
        res.status = 201;
        return;
    }
    RETURN_RES(err, 409);
}

void GameServer::Logout(const Request &req, Response &res) {
    SESSION_ID;
    std::string err = "Logged out successfully.";
    memory.TryLogoutPlayer(sessionID, &err);
    DeleteCookie(SESSION_COOKIE, req, res);
    RETURN_RES(err, 200);
}

void GameServer::ViewMe(const Request &req, Response &res) {
    SESSION_ID;
    std::string err;
    const auto p = memory.TryGetPlayer(sessionID, &err);
    if (!p) RETURN_RES(err, 401);

    nlohmann::json teamJ = nlohmann::json::array();
    for (const auto &t: *p->Teams()) {
        if (!t) {
            teamJ.push_back(nullptr);
            continue;
        }
        nlohmann::json monJ = nlohmann::json::array();
        for (const auto &m: t->Monsters()) {
            if (!m) monJ.push_back(nullptr);
            else
                monJ.push_back({
                    {"Type", MonsterDescriptions.at(m->Type).TypeAsString},
                    {"Level", m->GetStatDict()->Get(Stat::Level)}
                });
        }
        teamJ.push_back({
            {"Name", t->Name},
            {"ID", t->ID},
            {"Monsters", monJ}
        });
    }
    const nlohmann::json j{
        {"Username", p->Username},
        {"Score", p->GetScore()},
        {"Teams", teamJ}
    };
    RETURN_RES(j.dump(), 200)
}

void GameServer::CreateTeam(const Request &req, Response &res) {
    REQUIRE_PARAMS("Name")
    const std::string name = req.get_param_value("Name");
    std::string err;
    SESSION_ID
    const int id = memory.TryGetNewTeamID(sessionID, name, &err);
    if (id == -1) RETURN_RES(err, 400)
    RETURN_RES(std::to_string(id), 201);
}

void GameServer::DeleteTeam(const Request &req, Response &res) {
    REQUIRE_PARAMS("ID");
    const int id = std::stoi(req.get_param_value("ID"));
    std::string err;
    SESSION_ID
    if (memory.TryDeleteTeam(sessionID, id, &err)) {
        res.status = 201;
        return;
    }
    RETURN_RES(err, 400)
}

void GameServer::ViewTeam(const Request &req, Response &res) {
    REQUIRE_PARAMS("ID")
    const int id = std::stoi(req.get_param_value("ID"));
    std::string err;
    SESSION_ID
    const auto team = memory.TryGetTeam(id, &err);
    if (!team) RETURN_RES(err, 400);
    RETURN_RES(team->ToJson().dump(), 200);
}

void GameServer::CreateMonster(const Request &req, Response &res) {
}

void GameServer::DeleteMonster(const Request &req, Response &res) {
}

void GameServer::LevelMonster(const Request &req, Response &res) {
}

void GameServer::ViewMonster(const Request &req, Response &res) {
}

void GameServer::Run() {
    server.listen("0.0.0.0", 8080);
}
