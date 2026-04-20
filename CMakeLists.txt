cmake_minimum_required(VERSION 4.1)
project(ASCII_Clash)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})
find_package(SQLite3 REQUIRED)
add_executable(ASCII_Clash main.cpp
        Config.h
        GameLogic/Monsters/MathHelpers.h
        GameLogic/Fighting/NestedLogger.h
        GameLogic/Monsters/Stats.h
        GameLogic/Fighting/Fight.h
        Database/DBManager.cpp
        Database/DBManager.h
        Server/GameServer.cpp
        Server/GameServer.h
        Server/PlayerSession.h
        Server/PlayerSession.cpp
        GameLogic/Monsters/Descriptions.h
        Server/ServerHelpers.h
        GameLogic/Monsters/StatDict.h
        GameLogic/Fighting/NestedLogger.cpp
        GameLogic/Fighting/LogEntry.cpp
        GameLogic/Fighting/SinglePlayer/SingleplayerRun.cpp
        GameLogic/Fighting/SinglePlayer/SingleplayerRun.h
        GameLogic/Fighting/Fight.cpp
        GameLogic/Monsters/Monsterbase.h
        GameLogic/Monsters/Monster.cpp
        GameLogic/Monsters/MonsterFactory.h
)
target_link_libraries(ASCII_Clash SQLite::SQLite3)