//
// Created by yifei on 12/12/2025.
//

#include "BookManager.h"

#include <cstring>

BookManager::BookManager(Persistence &db): db(db) {
    current.hasSelect = false;
}

std::vector<int> BookManager::show(const std::string &key, const std::string &field) {
    if (field == "ISBN") {
        return db.find(key);
    }
    else if (field == "NAME") {
        return db.find(key);
    }
    else if (field == "AUTHOR") {
        return db.find(key);
    }
    else if (field == "KEYWORD") {
        return db.find(key);
    }
    return {};
}

bool BookManager::select(const std::string &isbn) {
    std::vector<int> ids = db.find(isbn);
    if (ids.empty()) {
        current.hasSelect = false;
        return false;
    }
    strcpy(current.ISBN, isbn.c_str());
    current.hasSelect = true;
    return true;
}

bool BookManager::modify(int fieldFlag, const std::string &newValue) {
    if (!current.hasSelect) {
        return false;
    }

    //BookRecord book;
    //if (!db.getBook(current.ISBN, book)) {
    //    return false;
    //}

    //switch (fieldFlag)
}