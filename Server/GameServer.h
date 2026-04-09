#ifndef ASCII_CLASH_SERVERMANAGER_H
#define ASCII_CLASH_SERVERMANAGER_H
#include "PlayerSession.h"
#include "../Database/DBManager.h"
#include "../Imports/httplib.h"

inline const std::string ServerMountPath = "Server/HTML";

class GameServer {
public:
    explicit GameServer(DBManager &db);

    void Run();

private:
    const DBManager db;
    httplib::Server server;
    std::map<std::string, PlayerSession> sessions;

    void StartSession(const std::string *usr, const std::string *pwd, httplib::Response &res);

    std::optional<PlayerSession> GetSession(const httplib::Request &req, httplib::Response &res) const;

    static std::string RandomID();

    static std::optional<std::string> GetCookie(const std::string &key, const httplib::Request &req);
};


#endif
