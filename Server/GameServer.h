#ifndef ASCII_CLASH_SERVERMANAGER_H
#define ASCII_CLASH_SERVERMANAGER_H
#include "MemoryManager.h"
#include "../Imports/httplib.h"
#include "Database/SaveManager.h"

inline const std::string ServerMountPath = "Server/HTML";

using namespace Database;

class GameServer {
public:
    explicit GameServer(MemoryManager *memoryManager);

    void Run();

private:
    MemoryManager mm;
    httplib::Server server;
};

#endif
