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

// 某用户当前选中的书籍
struct SelectedBook {
    bool hasSelect;           // 是否已选书
    int offset = -1;
};

// 登录栈单元
struct AccountSession {
    UserRecord user;
    SelectedBook book;
};

// 图书信息
#pragma pack(push, 1)
struct BookRecord {
    char ISBN[20] = {""};
    char title[60] = {""};
    char author[60] = {""};
    char keyword_list[60] = {""};   // 关键字用 | 连接
    int stock = 0;
    double price = 0;
};
#pragma pack(pop)

struct FinanceRecord {
    double delta;             // +收入 / -支出
};

struct LogRecord {
    char message[128];
};
#endif //BOOKSTORE_TYPES_H