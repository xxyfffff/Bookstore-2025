//
// Created by yifei on 12/13/2025.
//

#ifndef BOOKSTORE_BOOKMANAGER_H
#define BOOKSTORE_BOOKMANAGER_H

#include <string>
#include <vector>
#include "Persistence.h"
#include "Types.h"

class BookManager {
public:
    explicit BookManager(Persistence &db);

    // 查询图书，按关键字/ISBN/作者/书名
    std::vector<int> show(const std::string &key, const std::string &field = "ISBN");

    // 选中图书
    bool select(const std::string &isbn);

    // 修改选中图书
    bool modify(int fieldFlag, const std::string &newValue);
    // fieldFlag: 0=ISBN, 1=书名, 2=作者, 3=关键字, 4=价格, 5=库存

    // 导入图书（增加库存）
    bool import(int quantity, double price);

    // 购买图书
    bool buy(int quantity);

    // 当前选中图书
    SelectedBook getSelected() const;

private:
    Persistence &db;
    SelectedBook current;
    std::vector<std::string> parseKeywords(const std::string &keywords);
};

#endif //BOOKSTORE_BOOKMANAGER_H
