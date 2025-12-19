//
// Created by yifei on 12/12/2025.
//

#include "BookManager.h"
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
void print(BookRecord b) {
    std::cout << b.ISBN << '\t'
                              << b.title << '\t'
                              << b.author << '\t'
                              << b.keyword_list << '\t'
                              << std::fixed << std::setprecision(2) << b.price << '\t'
                              << b.stock << '\n';
}

BookManager::BookManager(Persistence &db): db(db) {
    current.hasSelect = false;
}

bool BookManager::show(const std::string &field,
                       const std::string &key,
                       std::vector<BookRecord> &result) {
    result.clear();
    std::vector<int> ids;

    if (field.empty()) {
        ids = db.getAllBooks();
    }
    else {
        if (key.empty()) {
            //std::cerr << "empty\n";
            return false;
        }
        if (field == "KEYWORD" && key.find('|') != std::string::npos) {
            //std::cerr << "keyword\n";
            return false;
        }

        if (field == "ISBN") {
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
        }
        else if (field == "NAME") {
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
        }
        else if (field == "AUTHOR") {
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
        }
        else if (field == "KEYWORD") {
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
        }
        else {
            //std::cerr << "field invalid\n";
            return false; // 非法字段
        }
    }
    //db.debugDumpKeyword("before show keyword");
    for (auto id : ids) {
        BookRecord book;
        if (db.getBookByOffset(id, book)) {
            result.push_back(book);
        }
    }

    std::sort(result.begin(), result.end(),
              [](const BookRecord &a, const BookRecord &b) {
                  return std::strcmp(a.ISBN, b.ISBN) < 0;
              });

    return true;
}

bool BookManager::select(const std::string &ISBN) {
    if (ISBN.empty()) {
        return false;
    }

    std::vector<int> ids = db.findByISBN(ISBN);

    if (!ids.empty()) {
        current.hasSelect = true;
        current.offset = ids[0];
        //std::cerr << "Selected book exist\n";
        return true;
    }
    else {
        //std::cerr << "book doesn't exist\n";
    }

    BookRecord book;
    strncpy(book.ISBN, ISBN.c_str(), sizeof(book.ISBN));

    int offset = db.addBook(book);
    if (offset < 0) return false;
    db.insertISBN(ISBN, offset);

    current.hasSelect = true;
    current.offset = offset;
    db.updateBookByOffset(current.offset, book);

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
    if (!current.hasSelect || current.offset < 0) {
        return false;
    }

    BookRecord oldBook;
    if (!db.getBookByOffset(current.offset, oldBook)) return false;

    BookRecord newBook = oldBook;

    // 1. 修改字段
    switch (fieldFlag) {
        case 0: {
            if (newBook.ISBN == newValue) {
                return false;
            }
            std::vector<int> tmp = db.findByISBN(newValue);
            if (!tmp.empty()) {
                return false;
            }
            strcpy(newBook.ISBN, newValue.c_str());
            break;
        }
        case 1: strcpy(newBook.title, newValue.c_str()); break;
        case 2: strcpy(newBook.author, newValue.c_str()); break;
        case 3: strcpy(newBook.keyword_list, newValue.c_str()); break;
        case 4: newBook.price = std::stod(newValue); break;
        case 5: newBook.stock = std::stoi(newValue); break;
        default: return false;
    }

    // 2. 先删除旧索引
    db.removeISBN(oldBook.ISBN, current.offset);
    if (strlen(oldBook.title))
        db.removeName(oldBook.title, current.offset);
    if (strlen(oldBook.author))
        db.removeAuthor(oldBook.author, current.offset);
    if (strlen(oldBook.keyword_list)) {
        std::vector<std::string> Keywords = parseKeywords(oldBook.keyword_list);
        for (auto kw: Keywords) {
            if (!kw.empty()) {
                db.removeKeyword(kw.c_str(), current.offset);
            }
        }
    }

    // 3. 更新 book 本体
    db.updateBookByOffset(current.offset, newBook);

    // 4. 插入新索引
    db.insertISBN(newBook.ISBN, current.offset);
    if (strlen(newBook.title))
        db.insertName(newBook.title, current.offset);
    if (strlen(newBook.author))
        db.insertAuthor(newBook.author, current.offset);
    if (strlen(newBook.keyword_list)) {
        auto newKeywords = parseKeywords(newBook.keyword_list);
        for (auto &kw : newKeywords)
            db.insertKeyword(kw, current.offset);
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
    if (!current.hasSelect) {
        return false;
    }
    if (quantity <= 0 || totalCost < 0) {
        return false;
    }
    if (current.offset < 0) {
        return false;
    }

    BookRecord book;
    if (!db.getBookByOffset(current.offset, book)) {
        return false;
    }

    book.stock += quantity;
    db.updateBookByOffset(current.offset, book);
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
        return false;  // ISBN 不存在
    }

    int offset = ids[0];
    BookRecord book;
    if (!db.getBookByOffset(offset, book)) {
        return false;
    }

    if (book.stock < quantity) {
        return false;  // 库存不足
    }

    book.stock -= quantity;
    db.updateBookByOffset(offset, book);

    cost = book.price * quantity;
    return true;
}


std::vector<std::string> BookManager::parseKeywords(const std::string &keywords) {
    std::vector<std::string> res;
    std::string tmp;
    for (char c : keywords) {
        if (c == '|') {
            if (!tmp.empty()) res.push_back(tmp);
            tmp.clear();
        }
        else {
            tmp.push_back(c);
        }
    }
    if (!tmp.empty()) {
        res.push_back(tmp);
    }
    return res;
}

void BookManager::resetSelected() {
    current.hasSelect = false;
    current.offset = -1;
}