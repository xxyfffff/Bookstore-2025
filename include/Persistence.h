//
// Created by yifei on 12/13/2025.
//

#ifndef BOOKSTORE_PERSISTENCE_H
#define BOOKSTORE_PERSISTENCE_H

#include <string>
#include <vector>
#include "Types.h"
class Persistence {
public:
    Persistence();

    // 插入索引
    void insert(const std::string &key, int id);

    //删除索引
    void remove(const std::string &key, int id);

    // 查找 key 对应的所有 id
    std::vector<int> find(const std::string &key);

    // users.data
    bool getUser(const std::string &userID, UserRecord &out);
    bool addUser(const std::string &userID,
                          const std::string &password,
                          int privilege);
    bool updateUser(const std::string &userID,
                    const std::string &newPassword,
                    int newPrivilege = -1);
    bool deleteUser(const std::string &userID);
    bool userExists(const std::string &userID);

    // ISBN 索引
    void insertISBN(const std::string &isbn, int offset);
    void removeISBN(const std::string &isbn, int offset);
    std::vector<int> findByISBN(const std::string &isbn);
    // Name 索引
    void insertName(const std::string &name, int offset);
    void removeName(const std::string &name, int offset);
    std::vector<int> findByName(const std::string &name);
    // Author 索引
    void insertAuthor(const std::string &author, int offset);
    void removeAuthor(const std::string &author, int offset);
    std::vector<int> findByAuthor(const std::string &author);
    // Keyword 索引
    void insertKeyword(const std::string &keyword, int offset);
    void removeKeyword(const std::string &keyword, int offset);
    std::vector<int> findByKeyword(const std::string &keyword);
    // Book 文件
    int addBook(const BookRecord &book);
    bool getBookByOffset(int offset, BookRecord &book);
    void updateBookByOffset(int offset, const BookRecord &book);
    std::vector<int> getAllBooks();

private:
    class Impl;
    Impl *impl;
};

struct UserDiskRecord {
    char userID[64];
    char password[64];
    int privilege;
    bool valid;
};



#endif // BOOKSTORE_PERSISTENCE_H