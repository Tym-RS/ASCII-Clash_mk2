#include <thread>
#include <iostream>

#include "Database/SaveManager.h"
#include "Server/GameServer/GameServer.h"

int main(const int argc, const char *argv[]) {
    srand(time(nullptr));
    SaveManager sm(argc > 1 ? argv[1] : "Default");
    auto memoryTemp = std::make_unique<MemoryManager>(&sm);
    const auto memory = memoryTemp.get();
    GameServer server = GameServer(std::move(memoryTemp));

    std::thread serverThread([&server] { server.Run(); });

    std::string cmd;
    while (std::cin >> cmd) if (cmd == "stop") break;
    serverThread.join();
    return 0;
}

//ip addr | grep "inet "
