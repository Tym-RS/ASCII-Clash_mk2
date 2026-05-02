#include "GameServer.h"
#include "ServerHelpers.h"
#include "Endpoints.h"
#include "_SplashTexts.h"
#include "Database/TableDefs.h"

#define SESSION_ID const auto sessionID = GetCookie("session", req, res); if(sessionID.empty()) {RETURN_RES("Login required.", 401)}

using namespace httplib;
using namespace Database;
static const std::string SESSION_COOKIE = "session";


GameServer::GameServer(std::unique_ptr<MemoryManager> memory) : memory(std::move(memory)) {
    server.set_mount_point("/", Config::Server::MountPath);

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
    int limit = 25;
    if (req.has_param("limit")) limit = std::stoi(req.get_param_value("limit"));
    RETURN_RES(memory->GetLeaderboard(limit).dump(), 200);
}

void GameServer::GameInfo(const Request &req, Response &res) {
    static const auto gameDescription = GetGameDescriptionsJSON().dump();
    RETURN_RES(gameDescription, 200);
}

void GameServer::SplashText(const Request &req, Response &res) {
    RETURN_RES(GetSplashText(), 200);
}

void GameServer::Login(const Request &req, Response &res) {
    REQUIRE_PARAMS("username", "password");
    const std::string usr = req.get_param_value("username");
    const std::string pwd = req.get_param_value("password");
    std::string err;
    const auto sessionID = memory->GetNewSessionID(usr, Hash(pwd), &err);
    if (sessionID.empty()) RETURN_RES(err, 404);
    res.status = 200;
    SetCookie(SESSION_COOKIE, sessionID, req, res);
}

void GameServer::Register(const Request &req, Response &res) {
    REQUIRE_PARAMS("username", "password");
    const std::string usr = req.get_param_value("username");
    const std::string pwd = req.get_param_value("password");
    if (!std::regex_match(usr, Config::Player::UsernameRegex))
        RETURN_RES("Username must be 1–15 chars and may only contain letters, numbers, _ ( ) - : ; [ ] { }", 400);

    if (!std::regex_match(pwd, Config::Player::PasswordRegex))
        RETURN_RES("Password must be at least 5 characters and contain at least one special character", 400);

    std::string err;
    if (memory->TryRegisterPlayer(usr, Hash(pwd), &err)) {
        res.status = 201;
        return;
    }
    RETURN_RES(err, 409);
}

void GameServer::Logout(const Request &req, Response &res) {
    SESSION_ID;
    std::string err;
    memory->TryLogoutPlayer(sessionID, &err);
    DeleteCookie(SESSION_COOKIE, req, res);
    RETURN_RES(err, 200);
}

void GameServer::ViewMe(const Request &req, Response &res) {
    SESSION_ID;
    std::string err;
    const auto p = memory->TryGetPlayer(sessionID, &err);
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
            {COL(Teams, name), t->Name},
            {COL(Teams, ID), t->ID},
            {COL(Teams, fight_id), t->CurrentFightID()},
            {"in_fight", t->IsInFight()},
            {COL(Teams, monsters), monJ},
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
    SESSION_ID
    REQUIRE_PARAMS("name")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);

    const std::string name = req.get_param_value("name");
    if (!std::regex_match(name, Config::Player::UsernameRegex))
        RETURN_RES("Team name must be 1–15 chars and may only contain letters, numbers, _ ( ) - : ; [ ] { }", 400);
    const auto team = player->TryGetCreateNewTeam(name, &err);
    if (!team) RETURN_RES(err, 400)
    memory->Save(player);
    RETURN_RES(std::to_string(team->ID), 201);
}

void GameServer::DeleteTeam(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("ID");
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);

    if (!player->TryDeleteTeam(std::stoi(req.get_param_value("ID")), &err)) RETURN_RES(err, 400)
    memory->Save(player);
    res.status = 205;
}

void GameServer::ViewTeam(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("ID")
    const int id = std::stoi(req.get_param_value("ID"));
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404)
    const auto team = player->TryGetTeam(id, &err);
    if (!team) RETURN_RES(err, 404);
    RETURN_RES(team->ToJson().dump(), 200);
}

void GameServer::SetAutoFight(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("team_ID", "set_to")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404)
    const int id = std::stoi(req.get_param_value("team_ID"));
    const auto team = player->TryGetTeam(id, &err);
    if (!team) RETURN_RES(err, 404)
    team->AutoFight = std::stoi(req.get_param_value("set_to")) == 1;
    res.status = 204;
}

void GameServer::CreateMonster(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("team_ID", "type", "name")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404)
    const int id = std::stoi(req.get_param_value("team_ID"));
    const auto team = player->TryGetTeam(id, &err);
    if (!team) RETURN_RES(err, 404)
    const std::string name = req.get_param_value("name");
    if (!std::regex_match(name, Config::Player::UsernameRegex))
        RETURN_RES("Monster name must be 1–15 chars and may only contain letters, numbers, _ ( ) - : ; [ ] { }", 400);
    const MonsterType type = StringMonsterTypeMap.at(req.get_param_value("type"));
    const int newID = team->TryGetNewMonsterID(name, type, &err);
    if (newID == -1) RETURN_RES(err, 400);
    memory->Save(player);
    res.set_content(std::to_string(newID), "text/plain");
    res.status = 201;
}

void GameServer::DeleteMonster(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("team_ID", "monster_ID")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404)
    const auto team = player->TryGetTeam(std::stoi(req.get_param_value("team_ID")), &err);
    if (!team) RETURN_RES(err, 404)
    if (!team->TryDeleteMonster(std::stoi(req.get_param_value("monster_ID")), &err)) RETURN_RES(err, 404);
    memory->Save(player);
    res.status = 205;
}

void GameServer::LevelMonster(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("team_ID", "monster_ID")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);
    const auto team = player->TryGetTeam(std::stoi(req.get_param_value("team_ID")), &err);
    if (!team) RETURN_RES(err, 404);
    if (team->IsInFight()) RETURN_RES("Monsters canNOT be leveled while in a fight.", 403);
    const auto mon = team->TryGetMonster(std::stoi(req.get_param_value("monster_ID")), &err);
    if (!mon) RETURN_RES(err, 404);

    nlohmann::json data;
    try { data = nlohmann::json::parse(req.body); } catch (...) RETURN_RES("Invalid inputs", 400);

    int totalCost = 0;
    for (const auto &[key, value]: data.items()) {
        if (!StringStatMap.contains(key)) RETURN_RES("Stat '" + key + "' not found.", 404);
        if (!StatInfos[static_cast<int>(StringStatMap.at(key))].Levelable)
            RETURN_RES("Stat '" + key + "' canNOT be leveled.", 400);
        totalCost += value.get<int>();
    }
    if (totalCost == 0) RETURN_RES("No valid stats to level.", 400);
    if (totalCost > mon->GetStatDict()->Get(Stat::SkillPoints)) RETURN_RES("Not enough skill-points.", 400);

    for (const auto &[key, value]: data.items()) {
        const int count = value.get<int>();
        for (int i = 0; i < count; i++)
            if (!mon->GetStatDict()->TryLevel(StringStatMap.at(key), &err)) RETURN_RES(err, 400);
    }
    mon->Reset();
    memory->Save(player);
    res.status = 205;
}

void GameServer::ViewMonster(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("team_ID", "monster_ID");
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);
    const auto team = memory->TryGetTeam(std::stoi(req.get_param_value("team_ID")), &err);
    if (!team) RETURN_RES(err, 404);
    const auto mon = team->TryGetMonster(std::stoi(req.get_param_value("monster_ID")), &err);
    if (!mon) RETURN_RES(err, 404);
    RETURN_RES(mon->ToJson().dump(), 200);
}

void GameServer::CreateFightRequest(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("team_ID")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);
    const int teamID = std::stoi(req.get_param_value("team_ID"));
    const auto team = memory->TryGetTeam(teamID, &err);
    if (!team || !player->TryGetTeam(teamID, &err)) RETURN_RES(err, 404);
    if (!memory->TryOpenFightRequest(team, &err)) RETURN_RES(err, 403);
    res.status = 201;
}

void GameServer::GetFightRequests(const Request &req, Response &res) {
    memory->CleanFightRequests();
    RETURN_RES(memory->GetFightRequests().dump(), 200);
}

void GameServer::RetractFightRequest(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("team_ID")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);
    const int teamID = std::stoi(req.get_param_value("team_ID"));
    const auto team = memory->TryGetTeam(teamID, &err);
    if (!team || !player->TryGetTeam(teamID, &err)) RETURN_RES(err, 404);
    memory->RetractFightRequest(team, &err);
    res.status = 204;
}

void GameServer::StartFight(const Request &req, Response &res) {
    SESSION_ID
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);

    nlohmann::json data;
    try { data = nlohmann::json::parse(req.body); } catch (...) RETURN_RES("Invalid inputs", 400);

    std::vector<int> teamIDs;
    for (const auto &id: data) teamIDs.push_back(id.get<int>());
    bool playerOwnedTeamFound = false;
    for (const int id: teamIDs) {
        if (!player->TryGetTeam(id)) continue;
        playerOwnedTeamFound = true;
        break;
    }
    if (!playerOwnedTeamFound) RETURN_RES("You must own at least one team in the fight.", 403);

    const int fightID = memory->TryCreateFight(teamIDs, &err);
    if (fightID == -1) RETURN_RES(err, 400);

    memory->CleanFightRequests();
    RETURN_RES(std::to_string(fightID), 201);
}

void GameServer::ViewFight(const Request &req, Response &res) {
    REQUIRE_PARAMS("fight_ID")
    std::string err;
    const int fightID = std::stoi(req.get_param_value("fight_ID"));
    const auto fightJ = memory->TryGetFightJson(fightID, &err);
    if (fightJ.empty()) RETURN_RES(err, 404);
    RETURN_RES(fightJ.dump(), 200);
}

void GameServer::SubmitFightAction(const Request &req, Response &res) {
    SESSION_ID
    REQUIRE_PARAMS("fight_ID", "team_ID", "attacker_ID", "defender_ID")
    std::string err;
    const auto player = memory->TryGetPlayer(sessionID, &err);
    if (!player) RETURN_RES(err, 404);
    const auto team = player->TryGetTeam(std::stoi(req.get_param_value("team_ID")), &err);
    if (!team) RETURN_RES(err, 404);
    const auto fight = memory->TryGetFight(std::stoi(req.get_param_value("fight_ID")), &err);
    if (!fight) RETURN_RES(err, 404);
    const int attackerID = std::stoi(req.get_param_value("attacker_ID"));
    const int defenderID = std::stoi(req.get_param_value("defender_ID"));
    if (!fight->TryTakeTurn(team, attackerID, defenderID, &err)) RETURN_RES(err, 400);
    res.status = 205;
}

void GameServer::Run() {
    server.listen("0.0.0.0", 8080);
}

void GameServer::Stop() {
    server.stop();
    memory->Cleanup();
}
