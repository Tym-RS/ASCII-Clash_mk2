#include "GameServer.h"
#include <nlohmann/json.hpp>
#include "ServerHelpers.h"
#include "GameLogic/Monsters/Descriptions.h"

using namespace httplib;
using json = nlohmann::json;

GameServer::GameServer(const DBManager &db) : db(db), server(Server()) {
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

    server.Post("/gameInfo", [](const Request &req, Response &res) {
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
        if (!player) return;
        if (!req.has_param("id")) {
            res.set_content(player->GetMonsterJson().dump(), "application/json");
            return;
        }
        try {
            const int id = std::stoi(req.get_param_value("id"));
            res.set_content(player->GetMonsterJson(id).dump(), "application/json");
        } catch (const std::exception &e) {
            res.set_content("Monster ID's MUST be integers.", "text/plain");
            res.status = 401;
        }
    });

    server.Post("/levelMonster", [this](const Request &req, Response &res) {
        const auto player = GetSession(req, res);
        if (!player) return;

        if (!req.has_param("id")) {
            res.status = 401;
            res.set_content("No ID provided.", "text/plain");
            return;
        }
        int id = 0;
        try {
            id = std::stoi(req.get_param_value("id"));
        } catch (const std::exception &e) {
            res.set_content(e.what(), "text/plain");
            res.status = 401;
        }

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
        const auto session = GetSession(req, res);
        if (!session) return;
        DeleteSession(session);
        res.set_header("Set-Cookie", "session=; Max-Age=0; HttpOnly; Path=/; SameSite=Strict");
        res.set_redirect("/");
    });

    server.Post("/newSGRun", [this](const Request &req, Response &res) {
        // TODO
    });
}

void GameServer::Run() {
    server.listen("0.0.0.0", 8080);
}


void GameServer::StartSession(const std::string *usr, const std::string *pwd, Response &res) {
    PlayerSession *playerID = db.GetNewPlayerSession(usr, pwd);
    if (playerID == nullptr) {
        res.status = 401;
        res.set_content("login failed <", "text/plain");
        return;
    }
    sessions.emplace(playerID->SessionID, playerID);
    res.set_header("Set-Cookie", "session=" + playerID->SessionID + "; HttpOnly; Path=/; SameSite=Strict");
    res.status = 200;
}

PlayerSession *GameServer::GetSession(const Request &req, Response &res) const {
    const auto sessionID = GetCookie("session", req);
    PlayerSession *session = nullptr;

    if (sessionID.has_value() && sessions.contains(sessionID.value()))
        session = sessions.at(sessionID.value());

    if (session && session->IsActive()) {
        session->UpdateLastActivity();
        return session;
    }

    res.status = 401;
    res.set_content("Session expired.", "text/plain");
    res.set_redirect("/");
    return nullptr;
}

void GameServer::DeleteSession(const PlayerSession *session) {
    db.SavePlayer(session);
    sessions.erase(session->SessionID);
    delete session;
}
