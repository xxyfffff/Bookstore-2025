//
// Created by yifei on 12/17/2025.
//
#ifndef BOOKSTORE_TANSACTION_H
#define BOOKSTORE_TANSACTION_H
#include "Persistence.h"
#include "AccountManager.h"

class Transaction {
public:
    Transaction(Persistence &db, AccountManager &am);

    void add(double delta, const std::string &type);    // 正数收入，负数支出
    void showFinance(int count) const;
    void showFinanceAll() const;

    void reportFinance() const;

private:
    Persistence &db;
    AccountManager &am;
};
#endif //BOOKSTORE_TANSACTION_H