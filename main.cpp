#include <thread>
#include <iostream>
#include "Database/DBManager.h"
#include "Server/GameServer.h"

int main(const int argc, const char *argv[]) {
    Monster *human = CreateTypedMonster("Hans", 2, MonsterType::Human);
    Monster *orc = CreateTypedMonster("Morkgkg", 1, MonsterType::Orc);
    NestedLogger logger = NestedLogger();

    human->LogPtr = &logger;
    orc->LogPtr = &logger;

    for (int i = 0; i < 10; i++) {
        logger.Next("Round " + std::to_string(i));
        orc->Attack(human);
        human->Attack(orc);
    }
    //std::cout << logger.AsStr();
    std::cout << logger.AsJson().dump();
    return 0;

    const DBManager dbm(argc > 1 ? argv[1] : "Default");
    GameServer server = GameServer(dbm);

    std::thread serverThread([&server] { server.Run(); });

    std::string cmd;
    while (std::cin >> cmd) {
        if (cmd == "stop") break;
    }
    serverThread.join();
    return 0;
}

//ip addr | grep "inet "
