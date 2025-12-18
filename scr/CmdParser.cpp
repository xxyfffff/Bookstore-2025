//
// Created by yifei on 12/12/2025.
//

#include "CmdParser.h"

#include <iostream>

std::vector<std::string> CmdParser::tokenize(const std::string &line) {
    std::vector<std::string> res;
    std::string token;
    bool inQuote = false;

    for (char c : line) {
        if (c == '\"') {
            inQuote = !inQuote;
        }
        else if (c == ' ') {
            if (inQuote) {
                token += c;
            }
            else if (!token.empty()){
                res.push_back(token);
                token.clear();
            }
        }
        else {
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

bool CmdParser::validate(const std:: vector<std::string> &t, CommandType type) {
    int n = t.size();
    switch (type) {
        case CommandType::QUIT:
        case CommandType::EXIT:
        case CommandType::LOGOUT:
            return n == 1;
        case CommandType::REGISTER:
            return n == 4;
        case CommandType::SU:
            return n == 2 || n == 3;
        case CommandType::USERADD:
            return n == 5;
        case CommandType::DELETE:
            return n == 2;
        case CommandType::PASSWD:
            return n == 3 || n == 4;
        case CommandType::SHOW:
            return n >= 1;
        case CommandType::SELECT:
            return n == 2;
        case CommandType::MODIFY:
            return n >= 2;
        case CommandType::IMPORT:
            return n == 3;
        case CommandType::BUY:
            return n == 3;
        case CommandType::SHOWFINANCE:
            return n == 2 || n == 3;
        case CommandType::LOG:
            return n == 1;
        case CommandType::REPORTFINANCE:
            return n == 2;
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