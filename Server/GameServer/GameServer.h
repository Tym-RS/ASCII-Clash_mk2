#pragma once

#include "Server/MemoryManager/MemoryManager.h"
#include "Imports/httplib.h"
#include "Endpoints.h"

#define REQ_PARAMS const httplib::Request &req, httplib::Response &res


class GameServer final {
public:
    explicit GameServer(std::unique_ptr<MemoryManager> memory);

#define X(func) void func(REQ_PARAMS);
    ENDPOINTS
#undef X

    void Run();

    void Stop();

private:
    std::unique_ptr<MemoryManager> memory;
    httplib::Server server;
};
