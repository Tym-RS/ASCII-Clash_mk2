#include <thread>
#include <iostream>
#include "Database/DBManager.h"
#include "Server/GameServer.h"

int main(const int argc, const char *argv[]) {
    const DBManager dbm(argc > 1 ? argv[1] : "Default");
    GameServer server = GameServer(dbm);

    std::thread serverThread([&server]() {
        server.Run();
    });

    std::string cmd;
    while (std::cin >> cmd) {
        if (cmd == "stop") break;
    }
    serverThread.join();
    return 0;
}

//ip addr | grep "inet "
