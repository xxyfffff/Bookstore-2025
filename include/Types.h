//
// Created by yifei on 12/12/2025.
//

#ifndef BOOKSTORE_TYPES_H
#define BOOKSTORE_TYPES_H
// 用户账号
struct UserRecord {
    std::string userID;
    std::string password;
    int privilege; // visitor = 0, customer = 1, clerk = 3, admin = 7
};

// 登录栈单元
struct AccountSession {
    UserRecord user;
};

// 图书信息
struct BookRecord {
    char ISBN[20];
    char title[60];
    char author[60];
    char keyword_list[60];   // 关键字用 | 连接
    int quantity;
    double price;
};

// 某用户当前选中的书籍
struct SelectedBook {
    bool hasSelect;           // 是否已选书
    char ISBN[32];
};

// 倒排索引项：key -> ISBN
struct IndexEntry {
    char key[64];
    char ISBN[20];
};

// ISBN 主索引：ISBN -> file offset
struct ISBNIndex {
    char ISBN[20];
    long long offset;         // books.dat 偏移
};

struct FinanceRecord {
    double delta;             // +收入 / -支出
};

struct LogRecord {
    char message[128];
};

#endif //BOOKSTORE_TYPES_H