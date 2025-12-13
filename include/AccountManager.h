//
// Created by yifei on 12/12/2025.
//

#ifndef BOOKSTORE_ACCOUNTMANAGER_H
#define BOOKSTORE_ACCOUNTMANAGER_H

#include <string>
#include <vector>
#include "Types.h"
#include "Persistence.h"

class AccountManager {
public:
    explicit AccountManager(Persistence &persistence);

    bool registerUser(const std::string &userID,
                      const std::string &password);

    bool login(const std::string &userID,
               const std::string &password);

    bool su(const std::string &targetUserID);   // 无密码切换
    bool logout();

    bool changePassword(const std::string &userID,
                        const std::string &oldPwd,
                        const std::string &newPwd);

    bool addUser(const std::string &userID,
                 const std::string &password,
                 int privilege);

    bool deleteUser(const std::string &userID);

    bool isLoggedIn() const;
    int currentPrivilege() const;
    std::string currentUserID() const;

private:
    Persistence &db;
    std::vector<AccountSession> loginStack;
};

#endif //BOOKSTORE_ACCOUNTMANAGER_H