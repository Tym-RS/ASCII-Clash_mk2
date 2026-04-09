#ifndef ASCII_CLASH_DBMANAGER_H
#define ASCII_CLASH_DBMANAGER_H
#include <sqlite3.h>
#include <string>

inline const std::string DBPaths = "Database/DB_Files/";

class DBManager {
public:
    explicit DBManager(const std::string &dbName);

    bool TryRegisterPlayer(const std::string *username, const std::string *password, std::string *errorMsg) const;

    int GetPlayerID(const std::string *username, const std::string *password) const;

    ~DBManager();

private:
    void InitDB() const;

    sqlite3 *db{};
};


#endif
