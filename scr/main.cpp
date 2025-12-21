#include <iomanip>
#include "Persistence.h"
#include "CmdParser.h"
#include "AccountManager.h"
#include <iostream>
#include <string>
#include "BookManager.h"
#include "Transaction.h"

void printBook(BookRecord b) {
    std::cout << b.ISBN << '\t'
                              << b.title << '\t'
                              << b.author << '\t'
                              << b.keyword_list << '\t'
                              << std::fixed << std::setprecision(2) << b.price << '\t'
                              << b.stock << '\n';
}
int main() {
    Persistence db;

    //初始化
    UserRecord root;
    root.userID = "root";
    root.password = "sjtu";
    root.privilege = 7;
    if (!db.getUser("root", root)) {
        db.addUser("root", "sjtu", 7);
    }

    AccountManager am(db);{}
    BookManager bm(db, am);{}
    CmdParser parser;
    Transaction trans(db);
    std::string line;
    while (true) {
        if (!std::getline(std::cin, line) ){
            break;
        }

        ParsedCommand cmd = parser.parseLine(line);

        switch (cmd.type) {
            case CommandType::QUIT:
            case CommandType::EXIT:
                //std::cout << "Exiting...\n";
                return 0;

                /* ================= 帐户相关 ================= */
            case CommandType::REGISTER: {
                if (cmd.args.size() < 2) {
                    std::cout << "Invalid\n";
                    break;
                }
                std::string id = cmd.args[0];
                std::string pwd = cmd.args[1];
                if (am.addUser(id, pwd, 1)) {
                    //std::cout << "Success\n";
                } else {
                    std::cout << "Invalid\n";
                }
                break;
            }

            case CommandType::SU: {
                if (cmd.args.size() < 1) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (cmd.args.size() == 1) {
                    if (am.su(cmd.args[0])) {
                        bm.resetSelected();
                        //std::cout << "Switched to " << cmd.args[0] << "\n";
                    } else {
                        std::cout << "Invalid\n";
                    }
                    break;
                }
                if (cmd.args.size() == 2) {
                    if (am.login(cmd.args[0], cmd.args[1])) {
                        bm.resetSelected();
                        //std::cout << "Switched to " << cmd.args[0] << "\n";
                    } else {
                        std::cout << "Invalid\n";
                    }
                    break;
                }
                break;
            }

            case CommandType::LOGOUT: {
                if (am.logout()) {
                    //std::cout << "Logged out\n";
                } else {
                    std::cout << "Invalid\n";
                }
                break;
            }

            case CommandType::USERADD: {
                if (cmd.args.size() < 3) {
                    std::cout << "Invalid\n";
                    break;
                }
                std::string id = cmd.args[0];
                std::string pwd = cmd.args[1];
                int priv = std::stoi(cmd.args[2]);
                if (am.addUser(id, pwd, priv)) {
                    //std::cout << "Success\n";
                } else {
                    std::cout << "Invalid\n";
                }
                break;
            }

            case CommandType::DELETE: {
                if (cmd.args.size() < 1) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (am.deleteUser(cmd.args[0])) {
                    //std::cout << "Deleted\n";
                } else {
                    std::cout << "Invalid\n";
                }
                break;
            }

            case CommandType::PASSWD: {
                if (cmd.args.size() < 2) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (cmd.args.size() == 2) {
                    if (am.changePassword(cmd.args[0], "", cmd.args[1])) {
                        //std::cout << "Password changed\n";
                    } else {
                        std::cout << "Invalid\n";
                    }
                }
                if (cmd.args.size() == 3) {
                    if (am.changePassword(cmd.args[0], cmd.args[1], cmd.args[2])) {
                        //std::cout << "Password changed\n";
                    } else {
                        std::cout << "Invalid\n";
                    }
                }
                break;
            }

                /* ================= 图书相关 ================= */
            case CommandType::SELECT: {
                if (!am.isLoggedIn() || am.currentPrivilege() < 3) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (cmd.args.size() != 1) {
                    //std::cerr << "SelectCmd is too short\n";
                    std::cout << "Invalid\n";
                    break;
                }
                if (bm.select(cmd.args[0])) {
                    //std::cerr << "Select success\n";
                }
                else {
                    //std::cerr << "Select fail\n";
                    std::cout << "Invalid\n";
                }
                break;
            }

            case CommandType::MODIFY: {
                if (!am.isLoggedIn() || am.currentPrivilege() < 3) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (cmd.args.empty()) {
                    //std::cerr << "ModifyCmd is too short\n";
                    std::cout << "Invalid\n";
                    break;
                }

                bool ok = true;
                for (auto &arg : cmd.args) {
                    // arg 形如 -ISBN=xxx / -name=xxx ...
                    if (arg.size() < 2 || arg[0] != '-') {
                        //std::cerr << "ModifyCmd is invalid\n";
                        ok = false;
                        break;
                    }

                    auto pos = arg.find('=');
                    if (pos == std::string::npos) {
                       // std::cerr << "Cannot find =\n";
                        ok = false;
                        break;
                    }

                    std::string field = arg.substr(1, pos - 1);
                    std::string value = arg.substr(pos + 1);

                    int flag = -1;
                    if (field == "ISBN") flag = 0;
                    else if (field == "name") flag = 1;
                    else if (field == "author") flag = 2;
                    else if (field == "keyword") flag = 3;
                    else if (field == "price") flag = 4;
                    else if (field == "stock") flag = 5;

                    if (flag == -1 || !bm.modify(flag, value)) {
                        //std::cerr << "Field is invalid\n";
                        ok = false;
                        break;
                    }
                }

                if (!ok) {
                    std::cout << "Invalid\n";
                }
                break;
            }

            case CommandType::SHOW: {
                if (!am.isLoggedIn() || am.currentPrivilege() < 1) {
                    std::cout << "Invalid\n";
                    break;
                }
                std::vector<BookRecord> result;

                if (cmd.args.empty()) {
                    if (bm.show("", "", result)) {
                        //std::cerr << "ShowAll success\n";
                    }
                    else {
                        //std::cerr << "ShowAll fail\n";
                        std::cout << "Invalid\n";
                        break;
                    }
                }
                else if (cmd.args.size() == 1) {
                    auto &arg = cmd.args[0];
                    if (arg.size() < 2 || arg[0] != '-') {
                        ///std::cerr << "ShowCmd is invalid";
                        std::cout << "Invalid\n";
                        break;
                    }

                    auto pos = arg.find('=');
                    if (pos == std::string::npos) {
                        //std::cerr << "Cannot find =\n";
                        std::cout << "Invalid\n";
                        break;
                    }

                    std::string field = arg.substr(1, pos - 1);
                    std::string key = arg.substr(pos + 1);
                    //std::cerr << "to show:" << field << " " << key << std::endl;
                    if (!bm.show(field == "name" ? "NAME" :
                                 field == "author" ? "AUTHOR" :
                                 field == "keyword" ? "KEYWORD" :
                                 field == "ISBN" ? "ISBN" : "",
                                 key, result)) {
                        //std::cerr << "Show fail\n";
                        std::cout << "Invalid\n";
                        break;
                                 }
                    //std::cerr << "Show success\n";
                }
                else {
                    //std::cerr << "ShowCmd is too long";
                    std::cout << "Invalid\n";
                    break;
                }

                for (auto &b : result) {
                    printBook(b);
                }

                if (result.empty()) {
                    std::cout << '\n';
                }

                break;
            }

            case CommandType::BUY: {
                if (!am.isLoggedIn() || am.currentPrivilege() < 1) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (cmd.args.size() != 2) {
                    //std::cerr << "BuyCmd is too short\n";
                    std::cout << "Invalid\n";
                    break;
                }

                double cost = 0;
                try {
                    int quantity = std::stoi(cmd.args[1]);
                } catch (const std::invalid_argument&) {
                    std::cout << "Invalid\n";
                    break;
                } catch (const std::out_of_range&) {
                    std::cout << "Invalid\n";
                    break;
                }
                int quantity = std::stoi(cmd.args[1]);
                if (quantity < 0) {
                    std::cout << "Invalid\n";
                    break;
                }

                if (!bm.buy(cmd.args[0], quantity, cost)) {
                    //std::cerr << "Buy fail\n";
                    std::cout << "Invalid\n";
                }
                else {
                    std::cout << std::fixed << std::setprecision(2) << cost << '\n';
                    trans.add(cost);
                }
                break;
            }

            case CommandType::IMPORT:
                {if (!am.isLoggedIn() || am.currentPrivilege() < 3) {
                std::cout << "Invalid\n";
                break;
                }
                if (cmd.args.size() != 2) {
                    //std::cerr << "ImportCmd is too short\n";
                    std::cout << "Invalid\n";
                    break;
                }

                int quantity = std::stoi(cmd.args[0]);
                double totalCost = std::stod(cmd.args[1]);

                if (!bm.import(quantity, totalCost)) {
                    //std::cerr << "Import fail\n";
                    std::cout << "Invalid\n";
                    break;
                }
                trans.add(-totalCost);
                //std::cerr << "Import success\n";
                break;
            }
                /* ================= 日志相关 ================= */
            case CommandType::SHOWFINANCE: {
                if (!am.isLoggedIn() || am.currentPrivilege() < 7) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (cmd.args.size() > 2) {
                    std::cout << "Invalid\n";
                    break;
                }
                if (cmd.args.size() == 1) {
                    trans.showFinanceAll();
                    break;
                }
                if (cmd.args.size() == 2){
                    int cnt = 0;
                    for (auto c: cmd.args[1]) {
                        cnt = cnt * 10 + c - '0';
                    }
                    trans.showFinance(cnt);
                    break;
                }
            }

            default:
                std::cout << "Invalid\n";
                break;
        }
    }
    return 0;
}
