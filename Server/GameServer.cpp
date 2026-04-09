#include "GameServer.h"
#include <nlohmann/json.hpp>
#include "GameLogic/Monsters/Descriptions.h"
using namespace httplib;
using json = nlohmann::json;

GameServer::GameServer(DBManager &db) : db(db), server(Server()) {
    server.set_mount_point("/", ServerMountPath);

    server.Get(R"(/(.*))", [](const Request &req, Response &res) {
        const std::string folder = "Server/HTML/";
        std::string path = req.matches[1];
        if (path.empty()) path = "index.html";
        std::string fullPath = folder + path;
        if (!std::filesystem::exists(fullPath)) fullPath += ".html";
        if (std::filesystem::exists(fullPath)) {
            std::ifstream file(fullPath);
            std::stringstream ss;
            ss << file.rdbuf();
            res.set_content(ss.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("404 Not Found", "text/plain");
        }
    });

    server.Post("/gameInfo", [this](const Request &req, Response &res) {
        const json data = GetGameDescriptionsJSON();
        std::string key;
        if (req.has_param("key")) key = req.get_param_value("key");
        res.set_content(key.empty() ? data.dump() : data.at(key).dump(), "application/json");
    });

    server.Post("/login", [this](const Request &req, Response &res) {
        const std::string usr = req.get_param_value("username");
        const std::string pwd = req.get_param_value("password");
        StartSession(&usr, &pwd, res);
    });

    server.Post("/register", [this](const Request &req, Response &res) {
        std::string error;
        const std::string usr = req.get_param_value("username");
        const std::string pwd = req.get_param_value("password");
        if (this->db.TryRegisterPlayer(&usr, &pwd, &error)) {
            StartSession(&usr, &pwd, res);
            return;
        }
        res.status = 401;
        res.set_content(error, "text/plain");
    });

    server.Post("/getMonster", [this](const Request &req, Response &res) {
        const auto player = GetSession(req, res);
        if (!player.has_value()) return;

        std::string id;
        if (req.has_param("id")) id = req.get_param_value("id");
        res.set_content(player->GetMonsterJson(id).dump(), "application/json");
        res.status = 200;
    });

    server.Post("/levelMonster", [this](const Request &req, Response &res) {
        const auto player = GetSession(req, res);
        if (!player.has_value()) return;

        if (!req.has_param("id")) {
            res.status = 401;
            res.set_content("No ID provided.", "text/plain");
            return;
        }
        const std::string id = req.get_param_value("id");

        json data;
        try { data = json::parse(req.body); } catch (const std::exception &e) {
            res.status = 401;
            res.set_content("Invalid level JSON.", "text/plain");
        }
        std::string err;
        if (player->TryLevelMonster(id, data, &err)) {
            res.status = 200;
            return;
        }
        res.status = 401;
        res.set_content(err, "text/plain");
    });

    server.Post("/logout", [this](const Request &req, Response &res) {
        auto session = GetSession(req, res);
        if (!session.has_value()) {
            // TODO
        }
    });
    server.Post("/newSGRun", [this](const Request &req, Response &res) {
        // TODO
    });
}

void GameServer::Run() {
    server.listen("0.0.0.0", 8080);
}


void GameServer::StartSession(const std::string *usr, const std::string *pwd, Response &res) {
    const int playerID = db.GetPlayerID(usr, pwd);
    if (playerID == -1) {
        res.status = 401;
        res.set_content("login failed <", "text/plain");
        return;
    }
    const std::string sessionID = RandomID();
    sessions.emplace(sessionID, PlayerSession(playerID, sessionID));
    res.set_header("Set-Cookie", "session=" + sessionID + "; HttpOnly; Path=/; SameSite=Strict");
    res.status = 200;
}

std::optional<PlayerSession> GameServer::GetSession(const Request &req, Response &res) const {
    const auto sessionID = GetCookie("session", req);
    if (!sessionID.has_value() || !sessions.contains(sessionID.value())) {
        res.status = 401;
        res.set_content("Session expired.", "text/plain");
        return std::nullopt;
    }
    PlayerSession session = sessions.at(sessionID.value());
    session.UpdateLastActivity();
    return session;
}
