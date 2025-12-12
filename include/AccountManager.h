//
// Created by yifei on 12/12/2025.
//

#ifndef BOOKSTORE_ACCOUNTMANAGER_H
#define BOOKSTORE_ACCOUNTMANAGER_H

class AccountManager {
public:
    AccountManager(Persistence &db);

    bool registerUser(const std::string &id, const std::string &pwd);
    bool login(const std::string &id, const std::string &pwd);
    bool loginDirect(const std::string &id);        // su
    void logout();
    bool passwd(const std::string &id,
                const std::string &oldpwd,
                const std::string &newpwd);
    bool useradd(const std::string &id, int privilege, const std::string &pwd);
    bool deleteUser(const std::string &id);

    int currentPrivilege() const;
    std::string currentUser() const;

private:
    Persistence &db;
    std::vector<AccountSession> stack;  // 登录栈
};

#endif //BOOKSTORE_ACCOUNTMANAGER_H