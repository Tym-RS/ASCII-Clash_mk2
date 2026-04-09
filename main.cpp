#include "Database/DBManager.h"
#include "Server/GameServer.h"

int main(const int argc, const char *argv[]) {
    DBManager dbm(argc > 1 ? argv[1] : "Default");
    GameServer server = GameServer(dbm);
    server.Run();
    return 0;
}

//ip addr | grep "inet "
