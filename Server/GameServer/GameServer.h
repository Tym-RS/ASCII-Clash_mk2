#ifndef ASCII_CLASH_SERVERMANAGER_H
#define ASCII_CLASH_SERVERMANAGER_H

#include "../MemoryManager/MemoryManager.h"
#include "../../Imports/httplib.h"
#include "Database/SaveManager.h"
#include "Endpoints.h"

#define REQ_PARAMS const httplib::Request &req, httplib::Response &res

inline const std::string ServerMountPath = "Server/HTML";

using namespace Database;

class GameServer final {
public:
    explicit GameServer(SaveManager *saveManager);

#define X(func) void func(REQ_PARAMS);
    ENDPOINTS
#undef X

    void Run();

private:
    SaveManager *db;
    MemoryManager memory;
    httplib::Server server;
};

#endif
