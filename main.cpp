#include <thread>
#include <iostream>

#include "Database/SaveManager.h"
#include "Server/GameServer.h"

int main(const int argc, const char *argv[]) {
    const SaveManager sm(argc > 1 ? argv[1] : "Default");
    //GameServer server = GameServer();

    //std::thread serverThread([&server] { server.Run(); });

    std::string cmd;
    while (std::cin >> cmd) {
        if (cmd == "stop") break;
    }
    //serverThread.join();
    return 0;
}

//ip addr | grep "inet "
