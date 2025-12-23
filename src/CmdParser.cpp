//
// Created by yifei on 12/12/2025.
//

#include "CmdParser.h"
#include <cctype>
#include <climits>
#include <cstdint>
#include <iostream>

static bool isVisibleASCII(char c) {
    return c >= 32 && c <= 126;
}

static bool isAllDigit(const std::string &s) {
    if (s.empty()) return false;
    for (char c: s) {
        if (!std::isdigit(c)) return false;
    }
    return true;
}

static bool isValidUserID(const std::string &s) {
    if (s.empty() || s.size() > 30) return false;
    for (char c: s) {
        if (!(std::isalnum(c) || c == '_')) return false;
    }
    return true;
}

static bool isValidUsername(const std::string &s) {
    if (s.empty() || s.size() > 30) return false;
    for (char c: s)
        if (!isVisibleASCII(c)) return false;
    return true;
}

static bool isValidPrivilege(const std::string &s) {
    return s.size() == 1 && std::isdigit(s[0]);
}

static bool isValidISBN(const std::string &s) {
    if (s.empty() || s.size() > 20) return false;
    for (char c: s)
        if (!isVisibleASCII(c)) return false;
    return true;
}

static bool isValidBookText(const std::string &s) {
    if (s.empty() || s.size() > 60) return false;
    for (char c: s) {
        if (!isVisibleASCII(c) || c == '"') return false;
    }
    return true;
}

static bool isValidKeyword(const std::string &s) {
    if (s.empty() || s.size() > 60) return false;

    bool lastWasSep = true; // 防止开头就是 |
    for (char c: s) {
        if (c == '|') {
            if (lastWasSep) return false; // 连续 || 或 | 开头
            lastWasSep = true;
        } else {
            if (!isVisibleASCII(c) || c == '"') return false;
            lastWasSep = false;
        }
    }
    return !lastWasSep; // 不能以 | 结尾
}

static bool isValidQuantity(const std::string &s) {
    if (s.empty()) return false;

    // 不能有前导 0
    if (s.size() > 1 && s[0] == '0') return false;

    for (char c: s)
        if (!std::isdigit(c)) return false;

    try {
        long long v = std::stoll(s);
        return v > 0 && v <= INT32_MAX;
    } catch (...) {
        return false;
    }
}

static bool isValidPrice(const std::string &s) {
    if (s.empty()) return false;

    int n = s.size();
    int dotPos = -1;

    for (int i = 0; i < n; ++i) {
        char c = s[i];
        if (c == '.') {
            if (dotPos != -1) return false; // 多个 .
            dotPos = i;
        } else if (!std::isdigit(c)) {
            return false;
        }
    }

    // 不能是 ".1" 或 "1."
    if (dotPos == 0 || dotPos == n - 1)
        return false;

    // 小数位数不超过 2
    if (dotPos != -1 && n - dotPos - 1 > 2)
        return false;

    if (s[0] == '0' && n > 1 && s[1] != '.')
        return false;

    try {
        double v = std::stod(s);
        return v >= 0;
    } catch (...) {
        return false;
    }
}



std::vector<std::string> CmdParser::tokenize(const std::string &line) {
    std::vector<std::string> res;
    std::string token;
    bool inQuote = false;

    for (char c: line) {
        if (c == '\"') {
            inQuote = !inQuote;
        } else if (c == ' ') {
            if (inQuote) {
                token += c;
            } else if (!token.empty()) {
                res.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        res.push_back(token);
    }
    return res;
}

CommandType CmdParser::determineType(const std::vector<std::string> &tokens) {
    if (tokens.empty()) {
        return CommandType::INVALID;
    }

    const std::string &cmd = tokens[0];
    if (cmd == "quit") return CommandType::QUIT;
    if (cmd == "exit") return CommandType::EXIT;
    if (cmd == "register") return CommandType::REGISTER;
    if (cmd == "su") return CommandType::SU;
    if (cmd == "logout") return CommandType::LOGOUT;
    if (cmd == "useradd") return CommandType::USERADD;
    if (cmd == "delete") return CommandType::DELETE;
    if (cmd == "passwd") return CommandType::PASSWD;
    if (cmd == "show") {
        if (tokens.size() >= 2 && tokens[1] == "finance") {
            return CommandType::SHOWFINANCE;
        }
        return CommandType::SHOW;
    }
    if (cmd == "buy") return CommandType::BUY;
    if (cmd == "select") return CommandType::SELECT;
    if (cmd == "modify") return CommandType::MODIFY;
    if (cmd == "import") return CommandType::IMPORT;
    if (cmd == "log") return CommandType::LOG;
    if (cmd == "report") {
        if (tokens.size() < 2) return CommandType::INVALID;
        const std::string &cmd2 = tokens[1];
        if (cmd2 == "finance") {
            return CommandType::REPORTFINANCE;
        }
        if (cmd2 == "employee") {
            return CommandType::REPORTEMLOYEE;
        }
    }
    return CommandType::INVALID;
}

bool CmdParser::validate(const std::vector<std::string> &t, CommandType type) {
    int n = t.size();
    switch (type) {
        case CommandType::QUIT:
        case CommandType::EXIT:
        case CommandType::LOGOUT:
            return n == 1;

        case CommandType::REGISTER:
            //std::cout << "REGISTER validate: n=" << n
            //  << " id1=" << t[1] << " id2=" << t[2]
            //  << " username=" << t[3] << "\n";
            //std::cout <<isValidUserID(t[1]) <<
            //      isValidUserID(t[2]) <<
            //       isValidUsername(t[3]);
            return (n == 4 &&
                    isValidUserID(t[1]) &&
                    isValidUserID(t[2]) &&
                    isValidUsername(t[3]));

        case CommandType::SU:
            if (n == 2) {
                return isValidUserID(t[1]);
            }
            if (n == 3) {
                return isValidUserID(t[1]) && isValidUserID(t[2]);
            }
            return false;

        case CommandType::USERADD:
            return n == 5 &&
                   isValidUserID(t[1]) &&
                   isValidUserID(t[2]) &&
                   isValidPrivilege(t[3]) &&
                   isValidUsername(t[4]);

        case CommandType::DELETE:
            return n == 2;

        case CommandType::PASSWD:
            if (n == 3) {
                return isValidUserID(t[1]) && isValidUserID(t[2]);
            }
            if (n == 4) {
                return isValidUserID(t[1]) &&
                       isValidUserID(t[2]) &&
                       isValidUserID(t[3]);
            }
            return false;

        case CommandType::SHOW: {
            if (n == 1) return true;
            if (n != 2) return false;

            auto pos = t[1].find('=');
            if (pos == std::string::npos) return false;

            std::string key = t[1].substr(1, pos - 1);
            std::string val = t[1].substr(pos + 1);

            if (key == "ISBN") return isValidISBN(val);
            if (key == "name") return isValidBookText(val);
            if (key == "author") return isValidBookText(val);
            if (key == "keyword") return isValidKeyword(val);

            return false;
        }

        case CommandType::SELECT:
            return n == 2 && isValidISBN(t[1]);

        case CommandType::MODIFY:
            if (n < 2) return false;
            for (int i = 1; i < n; i++) {
                auto &a = t[i];
                if (a.size() < 3 || a[0] != '-') return false;
                auto pos = a.find('=');
                if (pos == std::string::npos) return false;

                std::string key = a.substr(1, pos - 1);
                std::string val = a.substr(pos + 1);

                if (key == "ISBN" && !isValidISBN(val)) return false;
                if (key == "name" && !isValidBookText(val)) return false;
                if (key == "author" && !isValidBookText(val)) return false;
                if (key == "keyword" && !isValidKeyword(val)) return false;
                if (key == "price" && !isValidPrice(val)) return false;
                if (key == "stock" && !isValidQuantity(val)) return false;
            }
            return true;

        case CommandType::IMPORT:
            return n == 3 &&
                   isValidQuantity(t[1]) &&
                   isValidPrice(t[2]);

        case CommandType::BUY:
            return n == 3 &&
                   isValidISBN(t[1]) &&
                   isValidQuantity(t[2]);

        case CommandType::SHOWFINANCE:
            if (n == 2) return true;
            if (n == 3) return isAllDigit(t[2]);
            return false;

        case CommandType::LOG:
            return n == 1;

        case CommandType::REPORTFINANCE:
        case CommandType::REPORTEMLOYEE:
            return n == 2;

        default:
            return false;
    }
}


ParsedCommand CmdParser::parseLine(const std::string &line) {
    ParsedCommand res;

    auto tokens = tokenize(line);
    if (tokens.empty()) {
        return res;
    }

    CommandType type = determineType(tokens);
    if (!validate(tokens, type)) {
        res.type = CommandType::INVALID;
        return res;
    }
    res.type = type;

    res.args.assign(tokens.begin() + 1, tokens.end());
    return res;
}
