//
// Created by yifei on 12/12/2025.
//
#include "AccountManager.h"
#include <iostream>
#include "Persistence.h"
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
    loginStack.push_back({user});
    return true;
}

bool AccountManager::su(const std::string &targetUserID) {
    UserRecord target;
    if (target.privilege < currentPrivilege()) {
        loginStack.push_back({target});
        return true;
    }
    if (!db.getUser(targetUserID, target)) {
        return false;
    }
    loginStack.push_back({target});
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
    return db.deleteUser(userID);
}