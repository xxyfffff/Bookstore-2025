#include "Persistence.h"

#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <algorithm>
#include <iostream>

#include "Types.h"

namespace user{

    struct UserDiskRecord {
        char userID[64];
        char password[64];
        int privilege;
        bool valid;
    };

    UserDiskRecord toDisk(const UserRecord &u) {
        UserDiskRecord d{};
        strncpy(d.userID, u.userID.c_str(), 63);
        strncpy(d.password, u.password.c_str(), 63);
        d.privilege = u.privilege;
        d.valid = true;
        return d;
    }

    UserRecord fromDisk(const UserDiskRecord &d) {
        UserRecord u;
        u.userID = d.userID;
        u.password = d.password;
        u.privilege = d.privilege;
        return u;
    }

}

static const int NodeSpace = 1600;

struct Entry {
    char index[65];
    int value;
    Entry() = default;
    Entry(const std::string& idx, int val) {
        value = val;
        strncpy(index, idx.c_str(), 64);
        index[64] = '\0';
    }
    bool operator<(const Entry& other) const {
        int cmp = strcmp(index, other.index);
        if (cmp != 0) return cmp < 0;
        return value < other.value;
    }
    bool operator==(const Entry& other) const {
        return strcmp(index, other.index) == 0 && value == other.value;
    }
};

struct Node {
    int size;
    int next;
    Entry data[NodeSpace];

    Node() {
        size = 0;
        next = -1;
    }
    void setData(int idx, const Entry& l) {
        if (idx >= 0 && idx < NodeSpace) data[idx] = l;
    }

    Entry last_entry() const {
        if (size == 0) return Entry();
        return data[size-1];
    }
    void node_insert(const std::string& s, int val) {
        Entry l(s, val);
        if (size == 0 || l < data[0]) {
            for (int j = size; j >= 1; j--) data[j] = data[j-1];
            data[0] = l;
            size++;
            return;
        }
        for (int i = 0; i < size; i++) {
            if (l == data[i]) return;
            if (l < data[i]) {
                for (int j = size; j >= i+1; j--) data[j] = data[j-1];
                data[i] = l;
                size++;
                return;
            }
        }
        data[size] = l;
        size++;
    }
    void node_find(const std::string& s, std::vector<int>& res) {
        for (int i = 0; i < size; i++) {
            if (data[i].index == s) res.push_back(data[i].value);
        }
    }
    bool node_delete(const std::string& s, int val) {
        Entry l(s, val);
        for (int i = 0; i < size; i++) {
            if (data[i] == l) {
                for (int j = i+1; j < size; j++) data[j-1] = data[j];
                size--;
                return true;
            }
        }
        return false;
    }

    std::pair<Node, Node> split(int pos) {
        Node new_node;
        new_node.next = next;
        new_node.size = NodeSpace/2;
        size = NodeSpace/2;
        next = pos;
        for (int i = 0; i < NodeSpace/2; i++) {
            new_node.setData(i, data[NodeSpace/2 + i]);
        }
        return std::make_pair(*this, new_node);
    }
};

template<class T>
class MemoryRiver {
private:
    std::fstream file;
    std::string filename;
public:
    MemoryRiver() = default;
    MemoryRiver(const std::string& fn) : filename(fn) {}

    void initialise(const std::string& fn = "") {
        if (!fn.empty()) filename = fn;
        std::fstream check(filename);
        if (check.good()) {
            check.close();
            return;
        }
        file.open(filename, std::ios::out | std::ios::binary);
        int tmp = 0;
        file.write((char*)&tmp, sizeof(int));
        file.close();
    }

    int write(T& t) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::end);
        int index = file.tellp();
        file.write((char*)&t, sizeof(T));
        file.close();
        return index;
    }

    void read(T& t, int index) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            file.close();
            return;
        }
        file.seekg(index);
        file.read((char*)&t, sizeof(T));
        file.close();
    }

    void update(T& t, int index) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            file.close();
            return;
        }
        file.seekp(index);
        file.write((char*)&t, sizeof(T));
        file.close();
    }

    int end() {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0, std::ios::end);
        int index = file.tellp();
        file.close();
        return index;
    }

    void get_info(int &tmp) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekg(0);
        file.read((char*)&tmp, sizeof(int));
        file.close();
    }

    void write_info(int tmp) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(0);
        file.write((char*)&tmp, sizeof(int));
        file.close();
    }
};

class PieceNode {
private:
    MemoryRiver<Node> nodeRiver;
    int head;

public:
    PieceNode(const std::string &filename) {
        nodeRiver.initialise(filename);
        int tmp;
        nodeRiver.get_info(tmp);
        if (tmp == 0) {
            Node new_node;
            head = nodeRiver.write(new_node);
            nodeRiver.write_info(head);
        } else {
            head = tmp;
        }
    }

    void insert(const std::string& s, int val) {
        Entry l(s, val);
        int cnt = head;
        Node now;
        while (true) {
            nodeRiver.read(now, cnt);
            if (now.next == -1 || l < now.last_entry() || l == now.last_entry()) {
                now.node_insert(s, val);
                if (now.size >= NodeSpace) {
                    int pos = nodeRiver.end();
                    auto p = now.split(pos);
                    nodeRiver.write(p.second);
                    nodeRiver.update(p.first, cnt);
                } else {
                    nodeRiver.update(now, cnt);
                }
                return;
            }
            cnt = now.next;
        }
    }

    std::vector<int> find(const std::string& s) {
        int cnt = head;
        Node now;
        std::vector<int> res;
        while (cnt != -1) {
            nodeRiver.read(now, cnt);
            if (now.size > 0 && s > now.last_entry().index) {
                cnt = now.next;
                continue;
            }
            now.node_find(s, res);
            if (now.size > 0 && s < now.last_entry().index) break;
            cnt = now.next;
        }
        return res;
    }

    void remove(const std::string& s, int val) {
        int cnt = head;
        Node now;
        while (cnt != -1) {
            nodeRiver.read(now, cnt);
            if (now.size > 0 && s > now.last_entry().index) {
                cnt = now.next;
                continue;
            }
            if (now.node_delete(s, val)) {
                nodeRiver.update(now, cnt);
                return;
            }
            if (now.size > 0 && s < now.last_entry().index) break;
            cnt = now.next;
        }
    }

    void debug_dump(const std::string &tag) {
        std::cerr << "=== Dump index: " << tag << " ===\n";
        int cnt = head;
        Node now;
        int node_id = 0;
        while (cnt != -1) {
            nodeRiver.read(now, cnt);
            std::cerr << "Node " << node_id++
                      << " at " << cnt
                      << ", size=" << now.size
                      << ", next=" << now.next << "\n";
            for (int i = 0; i < now.size; ++i) {
                auto l = now.data[i];
                std::cerr << "  [" << i << "] "
                          << l.index
                          << " -> "
                          << l.value
                          << "\n";
            }
            cnt = now.next;
        }
        std::cerr << "=== End dump ===\n";
    }
};

class Persistence::Impl {
public:
    PieceNode index;
    PieceNode nameIndex;
    PieceNode authorIndex;
    PieceNode keywordIndex;
    std::fstream bookFile;
    int bookCount;

    std::fstream financeFile;
    int financeCnt;

    Impl() : index("index_isbn.dat"), // isbn主索引
            nameIndex("index_name.dat"),
            authorIndex("index_author.dat"),
            keywordIndex("index_keyword.dat"),
            bookCount(0),
            financeCnt(0){
        bookFile.open("books.dat", std::ios::in | std::ios::out | std::ios::binary);

        if (!bookFile.is_open()) {
            bookFile.open("books.dat",std::ios::out | std::ios::binary);
            bookFile.close();
            bookFile.open("books.dat",std::ios::in | std::ios::out | std::ios::binary);
        }

        bookFile.seekg(0, std::ios::end);
        bookCount = bookFile.tellg() / sizeof(BookRecord);

        financeFile.open("finance.dat",
            std::ios::in | std::ios::out | std::ios::binary);

        if (!financeFile.is_open()) {
            // 文件不存在，创建
            financeFile.open("finance.dat",
                std::ios::out | std::ios::binary);
            financeFile.close();

            financeFile.open("finance.dat",
                std::ios::in | std::ios::out | std::ios::binary);
        }

        financeFile.seekg(0, std::ios::end);
        financeCnt = financeFile.tellg() / sizeof(FinanceRecord);
    }
};

Persistence::Persistence() {
    impl = new Impl();

    impl->bookFile.clear();
    impl->bookFile.seekg(0, std::ios::beg);
    impl->bookFile.seekp(0, std::ios::end);

    impl->financeFile.clear();
    impl->financeFile.seekg(0, std::ios::beg);
    impl->financeFile.seekp(0, std::ios::end);
}

void Persistence::insert(const std::string &key, int id) {
    impl->index.insert(key, id);
}

void Persistence::remove(const std::string &key, int id) {
    impl->index.remove(key, id);
}

std::vector<int> Persistence::find(const std::string &key) {
    return impl->index.find(key);
}


bool Persistence::getUser(const std::string &userID,
                          UserRecord &out) {
    std::ifstream file("users.dat", std::ios::binary);
    if (!file) return false;

    user::UserDiskRecord d{};
    while (file.read((char*)&d, sizeof(d))) {
        if (!d.valid) continue;
        if (userID == d.userID) {
            out = fromDisk(d);
            return true;
        }
    }
    return false;
}

bool Persistence::addUser(const std::string &userID,
                          const std::string &password,
                          int privilege) {
    UserRecord tmp;
    if (getUser(userID, tmp)) return false; // 重名

    std::ofstream file("users.dat",std::ios::binary | std::ios::app);// 二进制，追加模式

    user::UserDiskRecord d{};
    strncpy(d.userID, userID.c_str(), 63);
    strncpy(d.password, password.c_str(), 63);
    d.privilege = privilege;
    d.valid = true;

    file.write((char*)&d, sizeof(d));
    return true;
}

bool Persistence::userExists(const std::string &userID) {
    UserRecord tmp;
    return getUser(userID, tmp);
}

bool Persistence::updateUser(const std::string &userID,
                             const std::string &newPassword,
                             int newPrivilege) {
    std::fstream file("users.dat",
        std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return false;

    user::UserDiskRecord d{};
    while (file.read((char*)&d, sizeof(d))) {
        if (!d.valid) continue;
        if (userID == d.userID) {
            if (!newPassword.empty())
                strncpy(d.password, newPassword.c_str(), 63);
            if (newPrivilege != -1)
                d.privilege = newPrivilege;

            file.seekp(-sizeof(d), std::ios::cur);
            file.write((char*)&d, sizeof(d));
            return true;
        }
    }
    return false;
}

bool Persistence::deleteUser(const std::string &userID) {
    std::fstream file("users.dat",
        std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return false;

    user::UserDiskRecord d{};
    while (file.read((char*)&d, sizeof(d))) {
        if (!d.valid) continue;
        if (userID == d.userID) {
            d.valid = false;
            file.seekp(-sizeof(d), std::ios::cur);
            file.write((char*)&d, sizeof(d));
            return true;
        }
    }
    return false;
}

void Persistence::updateBookByOffset(int offset, const BookRecord &book) {
    impl->bookFile.seekp(offset);
    impl->bookFile.write(reinterpret_cast<const char *>(&book),
                         sizeof(BookRecord));
    impl->bookFile.flush();
}

std::vector<int> Persistence::findByName(const std::string &name) {
    return impl->nameIndex.find(name);
}

std::vector<int> Persistence::findByAuthor(const std::string &author) {
    return impl->authorIndex.find(author);
}

std::vector<int> Persistence::findByKeyword(const std::string &keyword) {
    return impl->keywordIndex.find(keyword);
}

std::vector<int> Persistence::findByISBN(const std::string &isbn) {
    return impl->index.find(isbn);
}

bool Persistence::getBookByOffset(int offset, BookRecord &book) {
    if (offset < 0) {
        return false;
    }

    impl->bookFile.seekg(offset);
    impl->bookFile.read(reinterpret_cast<char *>(&book),
                        sizeof(BookRecord));
    return true;
}

std::vector<int> Persistence::getAllBooks() {
    std::vector<int> ids;
    for (int i = 0; i < impl->bookCount; ++i) {
        ids.push_back(i * sizeof(BookRecord));
    }
    return ids;
}

int Persistence::addBook(const BookRecord &book) {
    int offset = impl->bookCount * sizeof(BookRecord);

    impl->bookFile.seekp(offset);
    impl->bookFile.write(reinterpret_cast<const char *>(&book),
                         sizeof(BookRecord));

    impl->bookCount++;
    return offset;
}

void Persistence::insertISBN(const std::string &isbn, int offset) {
    impl->index.insert(isbn, offset);
}

void Persistence::removeISBN(const std::string &isbn, int offset) {
    impl->index.remove(isbn, offset);
}

void Persistence::insertName(const std::string &name, int offset) {
    impl->nameIndex.insert(name, offset);
}

void Persistence::removeName(const std::string &name, int offset) {
    impl->nameIndex.remove(name, offset);
}

void Persistence::insertAuthor(const std::string &author, int offset) {
    impl->authorIndex.insert(author, offset);
}

void Persistence::removeAuthor(const std::string &author, int offset) {
    impl->authorIndex.remove(author, offset);
}

void Persistence::insertKeyword(const std::string &keyword, int offset) {
    impl->keywordIndex.insert(keyword, offset);
}

void Persistence::removeKeyword(const std::string &keyword, int offset) {
    impl->keywordIndex.remove(keyword, offset);
}

bool Persistence::addFinanceRecord(double delta) {
    FinanceRecord d;
    d.delta = delta;
    impl->financeFile.seekp(0, std::ios::end);
    impl->financeFile.write(reinterpret_cast<const char*>(&d), sizeof(FinanceRecord));
    impl->financeFile.flush();

    impl->financeCnt++;
    return true;
}

bool Persistence::getFinanceRecord(std::vector<FinanceRecord> & frs, int cnt) {
    frs.clear();
    if (impl->financeCnt == 0) {
        return true;
    }

    int readCnt;
    if (cnt < 0 || cnt > impl->financeCnt) {
        return false;
    } else {
        readCnt = cnt;
    }

    int start = impl->financeCnt - readCnt;

    impl->financeFile.seekg(start * sizeof(FinanceRecord),
                            std::ios::beg);

    for (int i = 0; i < readCnt; ++i) {
        FinanceRecord fr;
        impl->financeFile.read(reinterpret_cast<char*>(&fr),
                                sizeof(FinanceRecord));
        frs.push_back(fr);
    }

    return true;
}

bool Persistence::getFinanceRecordAll(std::vector<FinanceRecord> & frs) {
    frs.clear();

    std::ifstream file("finance.dat", std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    FinanceRecord rec;
    while (file.read(reinterpret_cast<char*>(&rec), sizeof(FinanceRecord))) {
        frs.push_back(rec);
    }

    return true;
}

void Persistence::debugDumpKeyword(const std::string &tag) {
    impl->keywordIndex.debug_dump("keyword");
}