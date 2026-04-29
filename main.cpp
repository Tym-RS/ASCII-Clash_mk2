#include <thread>
#include <iostream>

#include "Database/SaveManager.h"
#include "Server/GameServer/GameServer.h"

int main(const int argc, const char *argv[]) {
    srand(time(nullptr));
    auto dbTemp = std::make_unique<SaveManager>(argc > 1 ? argv[1] : "Default");
    const auto data = dbTemp.get();
    auto memoryTemp = std::make_unique<MemoryManager>(std::move(dbTemp));
    const auto memory = memoryTemp.get();
    GameServer server = GameServer(std::move(memoryTemp));

    std::thread serverThread([&server] { server.Run(); });

    std::string cmd;
    while (std::cin >> cmd) {
        std::cout << std::string(100, '\n');
        if (cmd == "exit") break;
        if (cmd == "save") memory->Save();
        else if (cmd == "memory") memory->DebugDump();
        else if (cmd == "data") data->DebugDump();
        else if (cmd == "clean") memory->Cleanup();
    }
    serverThread.join();
    return 0;
}

//ip addr | grep "inet "


/*

ViewFight brauch n re-design

!!Fight dtor anders!!

*/
