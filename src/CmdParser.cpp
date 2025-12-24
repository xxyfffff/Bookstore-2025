//
// Created by yifei on 12/12/2025.
//

#include "CmdParser.h"
#include <cctype>
#include <climits>
#include <cstdint>
#include <iostream>
#include <unordered_set>

static bool isVisibleASCII(char c) {
    return c >= 32 && c <= 126;
}

static bool isAllDigit(const std::string &s) {
    if (s.empty()) {
        return false;
    }
    for (char c: s) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

static bool isValidUserID(const std::string &s) {
    if (s.empty() || s.size() > 30) {
        return false;
    }
    for (char c: s) {
        if (!(std::isalnum(c) || c == '_')) return false;
    }
    return true;
}

static bool isValidUsername(const std::string &s) {
    if (s.empty() || s.size() > 30) {
        return false;
    }
    for (char c: s)
        if (!isVisibleASCII(c)) return false;
    return true;
}

static bool isValidPrivilege(const std::string &s) {
    return s.size() == 1 && std::isdigit(s[0]);
}

static bool isValidISBN(const std::string &s) {
    if (s.empty() || s.size() > 20) {
        return false;
    }
    for (char c: s)
        if (!isVisibleASCII(c)) return false;
    return true;
}

static bool isValidBookText(const std::string &s) {
    if (s.empty() || s.size() > 60) {
        return false;
    }
    for (char c: s) {
        if (!isVisibleASCII(c) || c == '"') {
            return false;
        }
    }
    return true;
}

static bool isValidKeyword(const std::string &s) {
    if (s.empty() || s.size() > 60) {
        return false;
    }

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
    if (s.size() > 1 && s[0] == '0') {
        return false;
    }

    for (char c: s)
        if (!std::isdigit(c)) {
            return false;
        }

    try {
        long long v = std::stoll(s);
        return v > 0 && v <= INT32_MAX;
    } catch (...) {
        return false;
    }
}

static bool isValidPrice(const std::string &s) {
    // 长度和非空
    if (s.empty() || s.size() > 13) {
        return false;
    }

    // 字符集和小数点
    int dotPos = -1;
    for (int i = 0; i < (int)s.size(); ++i) {
        if (s[i] == '.') {
            if (dotPos != -1) return false; // 多个小数点
            dotPos = i;
        } else if (!isdigit(s[i])) {
            return false; // 非数字非小数点
        }
    }

    // 小数点位置合法性
    if (dotPos == 0) {
        return false;
    }

    // 前导0规则校验
    if (dotPos == -1) {
        // 无小数点（纯整数）
        if (s.size() > 1 && s[0] == '0') {
            return false;
        }
    } else {
        // 有小数点（小数）
        if (dotPos > 1 && s[0] == '0') {
            return false;
        }
        int fracLen = s.size() - dotPos - 1;
        if (fracLen == 0 || fracLen > 2) {// 小数部分不能为空或超过2位
            return false;
        }
    }

    // 数值合法性校验：转换为数值并判断>0
    double priceVal = std::stod(s);
    if (priceVal < 0) {
        return false;
    }

    // 整数部分长度限制
    int integerLen = (dotPos == -1) ? s.size() : dotPos;
    if (integerLen > 10) {
        return false;
    }

    return true;
}



std::vector<std::string> CmdParser::tokenize(const std::string &line) {
    std::vector<std::string> res;
    std::string token;
    bool inQuote = false;

    for (char c: line) {
        if (c == '\"') {
            token += c;   // 保留引号
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
            if (n == 1) {
                return true;
            }
            if (n != 2) {
                return false;
            }

            const std::string &arg = t[1];
            auto pos = arg.find('=');
            if (pos == std::string::npos) {
                return false;
            }

            std::string key = arg.substr(1, pos - 1);
            std::string rawVal = arg.substr(pos + 1);

            if (key == "ISBN") {
                return isValidISBN(rawVal);
            }

            // name / author / keyword 必须加引号
            if (key == "name" || key == "author" || key == "keyword") {
                if (rawVal.size() < 2) return false;
                if (rawVal.front() != '"' || rawVal.back() != '"') return false;

                // 去掉引号再验证
                std::string val = rawVal.substr(1, rawVal.size() - 2);

                if (key == "keyword") return isValidKeyword(val);
                return isValidBookText(val);
            }

            return false;
        }

        case CommandType::SELECT:
            return n == 2 && isValidISBN(t[1]);

        case CommandType::MODIFY: {
            if (n < 2) return false;
            std::unordered_set<std::string> seenKeys;
            for (int i = 1; i < n; ++i) {
                const std::string &arg = t[i];
                if (arg.size() < 3 || arg[0] != '-') return false;

                auto pos = arg.find('=');
                if (pos == std::string::npos) return false;

                std::string key = arg.substr(1, pos - 1);
                std::string rawVal = arg.substr(pos + 1);
                if (seenKeys.count(key)) return false;
                seenKeys.insert(key);
                if (key == "ISBN") {
                    if (!isValidISBN(rawVal)) return false;
                } else if (key == "price") {
                    if (!isValidPrice(rawVal)) return false;
                } else if (key == "stock") {
                    if (!isValidQuantity(rawVal)) return false;
                } else if (key == "name" || key == "author" || key == "keyword") {
                    // 必须带引号
                    if (rawVal.size() < 2) return false;
                    if (rawVal.front() != '"' || rawVal.back() != '"') return false;

                    std::string val = rawVal.substr(1, rawVal.size() - 2);

                    if (key == "keyword") {
                        if (!isValidKeyword(val)) return false;
                    } else {
                        if (!isValidBookText(val)) return false;
                    }
                } else {
                    return false; // 非法 key
                }
            }
            return true;
        }

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
    if (tokens.empty()) return res;

    CommandType type = determineType(tokens);
    if (!validate(tokens, type)) {
        res.type = CommandType::INVALID;
        return res;
    }

    res.type = type;

    // 传给 args 时去掉引号
    res.args.clear();
    for (size_t i = 1; i < tokens.size(); ++i) {
        std::string arg = tokens[i];
        auto pos = arg.find('=');
        if (pos != std::string::npos) {
            std::string key = arg.substr(0, pos+1);
            std::string val = arg.substr(pos+1);
            if (!val.empty() && val.front() == '"' && val.back() == '"') {
                val = val.substr(1, val.size() - 2); // 去掉引号
            }
            res.args.push_back(key + val);
        } else {
            res.args.push_back(arg);
        }
    }

    return res;
}
