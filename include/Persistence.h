//
// Created by yifei on 12/13/2025.
//

#ifndef BOOKSTORE_PERSISTENCE_H
#define BOOKSTORE_PERSISTENCE_H

#include <string>
#include <vector>
#include "Types.h"
class Persistence {
public:
    Persistence();

    // 插入索引
    void insert(const std::string &key, int id);

    //删除索引
    void remove(const std::string &key, int id);

    // 查找 key 对应的所有 id
    std::vector<int> find(const std::string &key);

    // users.data
    bool getUser(const std::string &userID, UserRecord &out);
    bool addUser(const std::string &userID,
                          const std::string &password,
                          int privilege);
    bool updateUser(const std::string &userID,
                    const std::string &newPassword,
                    int newPrivilege = -1);

    bool deleteUser(const std::string &userID);

    bool userExists(const std::string &userID);
private:
    class Impl;
    Impl *impl;
};

struct UserDiskRecord {
    char userID[64];
    char password[64];
    int privilege;
    bool valid;
};



#endif // BOOKSTORE_PERSISTENCE_H