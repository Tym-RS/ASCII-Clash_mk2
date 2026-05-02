#include <thread>
#include <iostream>

#include "Database/SaveManager.h"
#include "Server/GameServer/GameServer.h"

int main(const int argc, const char *argv[]) {
    srand(time(nullptr));
    auto dbTemp = std::make_unique<Database::SaveManager>(argc > 1 ? argv[1] : "Default");
    const auto data = dbTemp.get();
    auto memoryTemp = std::make_unique<MemoryManager>(std::move(dbTemp));
    const auto memory = memoryTemp.get();
    GameServer server = GameServer(std::move(memoryTemp));

    std::thread serverThread([&server] { server.Run(); });


    std::atomic running = true;
    std::thread cleanupThread([&] {
        while (running) {
            memory->Cleanup();
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
    });

    std::string cmd;
    while (std::cin >> cmd) {
        std::cout << std::string(100, '\n');
        if (cmd == "exit") break;
        if (cmd == "save") memory->Save();
        else if (cmd == "memory") memory->DebugDump();
        else if (cmd == "data") data->DebugDump();
        else if (cmd == "clean") memory->Cleanup();
    }
    running = false;
    cleanupThread.join();
    serverThread.join();
    server.Stop();
    return 0;
}

//ip addr | grep "inet "
