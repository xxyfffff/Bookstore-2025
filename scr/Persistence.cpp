#include "Persistence.h"

#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <algorithm>

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

} // anonymous namespace

static const int NodeSpace = 1600;

class Lines {
private:
    char index[65];
    int value;
public:
    Lines() {
        memset(index, 0, 65);
        value = 0;
    }
    Lines(const std::string& idx, int val) {
        value = val;
        strncpy(index, idx.c_str(), 64);
        index[64] = '\0';
    }
    std::string getIndex() const {
        return std::string(index);
    }
    int getValue() const {
        return value;
    }
    bool operator<(const Lines& other) const {
        int cmp = strcmp(index, other.index);
        if (cmp != 0) return cmp < 0;
        return value < other.value;
    }
    bool operator==(const Lines& other) const {
        return strcmp(index, other.index) == 0 && value == other.value;
    }
};

class Node {
private:
    int size;
    int next;
    Lines data[NodeSpace];
public:
    Node() {
        size = 0;
        next = -1;
    }
    void setSize(int Size) {
        size = Size;
    }
    int getSize() const {
        return size;
    }
    int getNext() const {
        return next;
    }
    void setNext(int n) {
        next = n;
    }
    Lines getData(int idx) const {
        if (idx >= 0 && idx < size) return data[idx];
        return Lines();
    }
    void setData(int idx, const Lines& l) {
        if (idx >= 0 && idx < NodeSpace) data[idx] = l;
    }
    Lines last_line() const {
        if (size == 0) return Lines();
        return data[size-1];
    }
    void node_insert(const std::string& s, int val) {
        Lines l(s, val);
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
            if (data[i].getIndex() == s) res.push_back(data[i].getValue());
        }
    }
    bool node_delete(const std::string& s, int val) {
        Lines l(s, val);
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
        new_node.setNext(next);
        new_node.setSize(NodeSpace/2);
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
        Lines l(s, val);
        int cnt = head;
        Node now;
        while (true) {
            nodeRiver.read(now, cnt);
            if (now.getNext() == -1 || l < now.last_line() || l == now.last_line()) {
                now.node_insert(s, val);
                if (now.getSize() >= NodeSpace) {
                    int pos = nodeRiver.end();
                    auto p = now.split(pos);
                    nodeRiver.write(p.second);
                    nodeRiver.update(p.first, cnt);
                } else {
                    nodeRiver.update(now, cnt);
                }
                return;
            }
            cnt = now.getNext();
        }
    }

    std::vector<int> find(const std::string& s) {
        int cnt = head;
        Node now;
        std::vector<int> res;
        while (cnt != -1) {
            nodeRiver.read(now, cnt);
            if (now.getSize() > 0 && s > now.last_line().getIndex()) {
                cnt = now.getNext();
                continue;
            }
            now.node_find(s, res);
            if (now.getSize() > 0 && s < now.last_line().getIndex()) break;
            cnt = now.getNext();
        }
        return res;
    }

    void remove(const std::string& s, int val) {
        int cnt = head;
        Node now;
        while (cnt != -1) {
            nodeRiver.read(now, cnt);
            if (now.getSize() > 0 && s > now.last_line().getIndex()) {
                cnt = now.getNext();
                continue;
            }
            if (now.node_delete(s, val)) {
                nodeRiver.update(now, cnt);
                return;
            }
            if (now.getSize() > 0 && s < now.last_line().getIndex()) break;
            cnt = now.getNext();
        }
    }
};

class Persistence::Impl {
public:
    PieceNode index;

    Impl() : index("bookstore_index.db") {}
};

Persistence::Persistence() {
    impl = new Impl();
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

    std::ofstream file("users.dat",
        std::ios::binary | std::ios::app);

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

