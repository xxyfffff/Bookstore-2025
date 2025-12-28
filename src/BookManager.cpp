//
// Created by yifei on 12/12/2025.
//

#include "BookManager.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <unordered_set>

AccountSession *BookManager::curSession() {
    return account.currentSession();
}

BookManager::BookManager(Persistence &db, AccountManager &account)
    : db(db), account(account) {
}

bool BookManager::show(const std::string &field,
                       const std::string &key,
                       std::vector<BookRecord> &result) {
    result.clear();
    std::vector<int> ids;

    if (field.empty()) {
        ids = db.getAllBooks();
    } else {
        if (key.empty()) {
            //std::cerr << "empty\n";
            return false;
        }
        if (field == "KEYWORD" && key.find('|') != std::string::npos) {
            //std::cerr << "keyword\n";
            return false;
        }

        if (field == "ISBN") {
            //exit(1);
            ids = db.findByISBN(key);
            /* 调试
            for (auto id : ids) {
                BookRecord book;
                if (db.getBookByOffset(id, book)) {
                    std::cerr << "test-ISBN:";
                    print(book);
                }
            }
            */
        } else if (field == "NAME") {
            //exit(1);
            ids = db.findByName(key);
            /* 调试
            for (auto id : ids) {
                BookRecord book;
                if (db.getBookByOffset(id, book)) {
                    std::cerr << "test-NAME:";
                    print(book);
                }
            }
            */
        } else if (field == "AUTHOR") {
            //exit(1);
            ids = db.findByAuthor(key);
            /* 调试
            for (auto id : ids) {
                BookRecord book;
                if (db.getBookByOffset(id, book)) {
                    std::cerr << "test-AUTHOR:";
                    print(book);
                }
            }
            */
        } else if (field == "KEYWORD") {
            //exit(1);
            if (key.empty()) return false;
            if (key.find('|') != std::string::npos) return false;
            ids = db.findByKeyword(key);
            /* 调试
            for (auto id : ids) {
                BookRecord book;
                if (db.getBookByOffset(id, book)) {
                    std::cerr << "test-KEYWORD:";
                    print(book);
                }
            }
            */
        } else {
            //std::cerr << "field invalid\n";
            return false; // 非法字段
        }
    }
    //db.debugDumpKeyword("before show keyword");
    for (auto id: ids) {
        BookRecord book;
        if (db.getBookByOffset(id, book)) {
            result.push_back(book);
        }
    }

    std::sort(result.begin(), result.end(),
              [](const BookRecord &a, const BookRecord &b) {
                  return std::strcmp(a.ISBN, b.ISBN) < 0;
              });
    db.addEmployeeRecord(curSession()->user.userID, "show " + field + " = " + key);
    return true;
}

bool BookManager::select(const std::string &ISBN) {
    if (ISBN.empty()) {
        return false;
    }
    auto *session = account.currentSession();
    if (!session) return false;

    std::vector<int> ids = db.findByISBN(ISBN);

    if (!ids.empty()) {
        session->book.hasSelect = true;
        session->book.offset = ids[0];
        //std::cerr << "Selected book exist\n";
        return true;
    } else {
        //std::cerr << "book doesn't exist\n";
    }

    BookRecord book{};
    memset(&book, 0, sizeof(BookRecord));
    strncpy(book.ISBN, ISBN.c_str(), sizeof(book.ISBN) - 1);

    int offset = db.addBook(book);
    if (offset < 0) return false;
    db.insertISBN(ISBN, offset);
    db.updateBookByOffset(offset, book);
    session->book.hasSelect = true;
    session->book.offset = offset;

    db.addEmployeeRecord(curSession()->user.userID, "select ISBN =" + ISBN);
    /* 调试
    BookRecord newbook;
    if (db.getBookByOffset(offset, newbook)) {
        std::cerr << "test-select:";
        print(newbook);
    }
    */
    return true;
}

bool BookManager::modify(int fieldFlag, const std::string &newValue) {
    auto *s = account.currentSession();
    if (!s || !s->book.hasSelect || s->book.offset < 0) return false;

    BookRecord oldBook;
    if (!db.getBookByOffset(s->book.offset, oldBook)) return false;

    BookRecord newBook = oldBook;

    // 1. 修改字段
    switch (fieldFlag) {
        case 0: {
            if (std::strcmp(newBook.ISBN, newValue.c_str()) == 0) {
                return false;
            }
            auto tmp = db.findByISBN(newValue);
            if (!tmp.empty()) {
                return false;
            }
            std::strncpy(newBook.ISBN, newValue.c_str(), sizeof(newBook.ISBN) - 1);
            db.addEmployeeRecord(curSession()->user.userID, "modify ISBN = " + newValue);
            break;
        }
        case 1: {
            strcpy(newBook.title, newValue.c_str());
            db.addEmployeeRecord(curSession()->user.userID, "modify title = " + newValue);
            break;
        }
        case 2: {
            strcpy(newBook.author, newValue.c_str());
            db.addEmployeeRecord(curSession()->user.userID, "modify author = " + newValue);
            break;
        }
        case 3: {
            if (newValue.empty()) return false;
            if (newValue.front() == '|' || newValue.back() == '|') return false;

            // 先解析新的关键字
            auto keywords = parseKeywords(newValue);

            // 检查是否有重复
            std::unordered_set<std::string> seen;
            for (auto &kw: keywords) {
                if (seen.count(kw)) {
                    return false; // 重复关键字，操作失败
                }
                seen.insert(kw);
            }

            // 检查每个关键字是否合法
            for (auto &kw: keywords) {
                if (kw.empty()) return false; // 空关键字非法
            }

            // 如果通过检查，拷贝到 book
            std::strncpy(newBook.keyword_list, newValue.c_str(), sizeof(newBook.keyword_list) - 1);
            db.addEmployeeRecord(curSession()->user.userID, "modify keyword = " + newValue);
            break;
        }
        case 4: {
            try {
                size_t idx = 0;
                double v = std::stod(newValue, &idx);
                if (idx != newValue.size() || v < 0) return false;
                newBook.price = v;
            } catch (...) {
                return false;
            }
            db.addEmployeeRecord(curSession()->user.userID, "modify price = " + newValue);
            break;
        }
        default: return false;
    }

    // 2. 先删除旧索引
    db.removeISBN(oldBook.ISBN, s->book.offset);
    if (strlen(oldBook.title))
        db.removeName(oldBook.title, s->book.offset);
    if (strlen(oldBook.author))
        db.removeAuthor(oldBook.author, s->book.offset);
    if (strlen(oldBook.keyword_list)) {
        std::vector<std::string> Keywords = parseKeywords(oldBook.keyword_list);
        for (auto kw: Keywords) {
            if (!kw.empty()) {
                db.removeKeyword(kw.c_str(), s->book.offset);
            }
        }
    }

    // 3. 更新 book 本体
    db.updateBookByOffset(s->book.offset, newBook);

    // 4. 插入新索引
    db.insertISBN(newBook.ISBN, s->book.offset);
    if (strlen(newBook.title))
        db.insertName(newBook.title, s->book.offset);
    if (strlen(newBook.author))
        db.insertAuthor(newBook.author, s->book.offset);
    if (strlen(newBook.keyword_list)) {
        auto newKeywords = parseKeywords(newBook.keyword_list);
        for (auto &kw: newKeywords)
            db.insertKeyword(kw, s->book.offset);
    }

    /* 调试
    BookRecord newbook;
    if (db.getBookByOffset(current.offset, newbook)) {
        std::cerr << "test-modify:";
        print(newbook);
    }
    */
    return true;
}

bool BookManager::import(int quantity, double totalCost) {
    auto *s = account.currentSession();
    if (!s || !s->book.hasSelect) return false;

    if (!s->book.hasSelect) {
        return false;
    }
    if (quantity <= 0 || totalCost <= 0) {
        return false;
    }
    if (s->book.offset < 0) {
        return false;
    }

    BookRecord book;
    if (!db.getBookByOffset(s->book.offset, book)) return false;

    book.stock += quantity;
    db.updateBookByOffset(s->book.offset, book);
    db.addEmployeeRecord(curSession()->user.userID, "import ISBN = " + std::string(book.ISBN) +
        " quantity = " + std::to_string(quantity));
    /* 调试
    BookRecord newbook;
    if (db.getBookByOffset(current.offset, newbook)) {
        std::cerr << "test-import:";
        print(newbook);
    }
    */
    return true;
}


bool BookManager::buy(const std::string &ISBN, int quantity, double &cost) {
    if (ISBN.empty() || quantity <= 0) {
        return false;
    }

    auto ids = db.findByISBN(ISBN);
    if (ids.empty()) {
        return false; // ISBN 不存在
    }

    int offset = ids[0];
    BookRecord book;
    if (!db.getBookByOffset(offset, book)) {
        return false;
    }
    if (book.stock < quantity) {
        return false; // 库存不足
    }

    book.stock -= quantity;
    db.updateBookByOffset(offset, book);

    cost = book.price * quantity;
    db.addEmployeeRecord(curSession()->user.userID, "buy ISBN = " + std::string(book.ISBN) +
        " quantity = " + std::to_string(quantity));
    return true;
}


std::vector<std::string> BookManager::parseKeywords(const std::string &keywords) {
    std::vector<std::string> res;
    std::string tmp;
    for (char c: keywords) {
        if (c == '|') {
            if (!tmp.empty()) res.push_back(tmp);
            tmp.clear();
        } else {
            tmp.push_back(c);
        }
    }
    if (!tmp.empty()) {
        res.push_back(tmp);
    }
    return res;
}

void BookManager::resetSelected() {
    auto *s = curSession();
    if (!s) return;
    s->book.hasSelect = false;
    s->book.offset = -1;
}
