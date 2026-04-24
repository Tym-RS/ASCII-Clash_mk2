cmake_minimum_required(VERSION 4.1)
project(ASCII_Clash)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})
find_package(SQLite3 REQUIRED)
add_executable(ASCII_Clash main.cpp
        Config.h
        GameLogic/MathHelpers.h
        GameLogic/Fighting/Log/NestedLogger.h
        GameLogic/Monsters/Stats/Stats.h
        GameLogic/Fighting/Fight.h
        Server/DEPRGameServer.cpp
        Server/GameServer.h
        Server/PlayerSession/PlayerSession.h
        Server/PlayerSession/PlayerSession.cpp
        GameLogic/Monsters/Descriptions.h
        Server/ServerHelpers.h
        GameLogic/Monsters/Stats/StatDict.h
        GameLogic/Fighting/Log/NestedLogger.cpp
        GameLogic/Fighting/Log/LogEntry.cpp
        GameLogic/Fighting/SinglePlayer/SingleplayerRun.cpp
        GameLogic/Fighting/SinglePlayer/SingleplayerRun.h
        GameLogic/Fighting/Fight.cpp
        GameLogic/Monsters/Monsterbase.h
        GameLogic/Monsters/Monster.cpp
        GameLogic/Monsters/CreateMonster.h
        Database/JsonSavable.h
        GameLogic/Monsters/Stats/StatDict.cpp
        Database/SaveManager.cpp
        Database/SaveManager.h
        Database/TableDefs.h
        GameLogic/Team.cpp
        GameLogic/Team.h
        Server/GameServer.cpp
        Server/MemoryManager.cpp
        Server/MemoryManager.h
        ErrHelper.h
)
target_link_libraries(ASCII_Clash SQLite::SQLite3)