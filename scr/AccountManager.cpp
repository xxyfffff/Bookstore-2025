//
// Created by yifei on 12/12/2025.
//
#include "AccountManager.h"
#include <iostream>
#include "Persistence.h"
AccountSession* AccountManager::currentSession() {
    if (loginStack.empty()) return nullptr;
    return &loginStack.back();
}

AccountManager::AccountManager(Persistence &db) : db(db) {}

bool AccountManager::isLoggedIn() const {
    return !loginStack.empty();
}

int AccountManager::currentPrivilege() const {
    if (loginStack.empty()) {
        return 0;
    }
    return loginStack.back().user.privilege;
}

std::string AccountManager::currentUserID() const {
    if (loginStack.empty()) {
        return "";
    }
    return loginStack.back().user.userID;
}

std::string AccountManager::currentPasswd() const {
    if (loginStack.empty()) {
        return "";
    }
    return loginStack.back().user.password;
}

bool AccountManager::login(const std::string &userID,
                           const std::string &password) {
    UserRecord user;
    if (!db.getUser(userID, user)) {
        //std::cerr << "user not exist\n";
        return false;
    }
    if (user.password != password) {
        //std::cerr << "wrong password\n";
        return false;
    }

    AccountSession s;
    s.user = user;
    s.book.hasSelect = false;
    s.book.offset = -1;
    loginStack.push_back(s);
    return true;
}

bool AccountManager::su(const std::string &targetUserID) {
    if (loginStack.empty()) return false;

    UserRecord target;
    if (!db.getUser(targetUserID, target)) {
        return false;
    }

    // 只能向下 su
    if (target.privilege >= currentPrivilege()) {
        return false;
    }

    AccountSession s;
    s.user = target;
    s.book.hasSelect = false;

    loginStack.push_back(s);
    return true;

}

bool AccountManager::logout() {
    if (loginStack.empty()) {
        return false;
    }
    loginStack.pop_back();
    return true;
}

bool AccountManager::changePassword(const std::string &userID,
                                    const std::string &oldPwd,
                                    const std::string &newPwd) {
    UserRecord user;
    if (!db.getUser(userID, user)) {
        return false;
    }
    bool isAdmin = (currentPrivilege() == 7);

    if (oldPwd.empty() && !isAdmin) {
        return false;
    }
    if (!isAdmin && user.password != oldPwd) {
        return false;
    }

    user.password = newPwd;
    return db.updateUser(userID, newPwd) ;
}

bool AccountManager::addUser(const std::string &userID,
                             const std::string &password,
                             int privilege) {

    if (currentPrivilege() < privilege && privilege != 1) {
        return false;
    }
    if (privilege >= 7) {
        return false;
    }

    UserRecord tmp;
    if (db.getUser(userID, tmp)) {
        return false;
    }

    return db.addUser(userID, password, privilege);
}

bool AccountManager::deleteUser(const std::string &userID) {
    if (currentPrivilege() != 7) {
        return false;
    }

    // 1. 用户必须存在
    UserRecord tmp;
    if (!db.getUser(userID, tmp)) {
        return false;
    }

    // 2. 用户不能处于登录状态（在 loginStack 中）
    for (auto &sess : loginStack) {
        if (sess.user.userID == userID) {
            return false;
        }
    }

    // 3. 删除
    return db.deleteUser(userID);
}