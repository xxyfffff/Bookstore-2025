//
// Created by yifei on 12/12/2025.
//

#ifndef BOOKSTORE_CMDPARSER_H
#define BOOKSTORE_CMDPARSER_H
#include <string>
#include <vector>

enum class CommandType {
    INVALID,
    // 基础指令
    QUIT,
    EXIT,
    // 账户系统指令
    SU,
    LOGOUT,
    REGISTER,
    PASSWD,
    USERADD,
    DELETE,
    // 图书系统指令
    SHOW,
    BUY,
    SELECT,
    MODIFY,
    IMPORT,
    // 日志系统指令
    SHOWFINANCE,
    LOG,
    REPORTFINANCE,
    REPORTEMLOYEE
};

struct ParsedCommand {
    CommandType type = CommandType::INVALID; // 初始化默认INVALID
    std::vector<std::string> args;

};

class CmdParser {
public:
    ParsedCommand parseLine(const std::string &line);

private:
    std::vector<std::string> tokenize(const std::string &line);
    CommandType determineType(const std::vector<std::string> &tokens);
    bool validate(const std::vector<std::string> &tokens, CommandType type);
};

#endif //BOOKSTORE_CMDPARSER_H