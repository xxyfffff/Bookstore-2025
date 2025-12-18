//
// Created by yifei on 12/17/2025.
//
#ifndef BOOKSTORE_TANSACTION_H
#define BOOKSTORE_TANSACTION_H
#include "Persistence.h"

class Transaction {
public:
    Transaction(Persistence &db);

    void add(double delta);    // 正数收入，负数支出
    void showFinance(int count) const;
    void showFinanceAll() const;

private:
    Persistence &db;
};
#endif //BOOKSTORE_TANSACTION_H