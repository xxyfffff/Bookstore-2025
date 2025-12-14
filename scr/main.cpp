#include "Persistence.h"
#include "CmdParser.h"
#include "AccountManager.h"
#include <iostream>
#include <string>
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
    CmdParser parser;

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
                        //std::cout << "Switched to " << cmd.args[0] << "\n";
                    } else {
                        std::cout << "Invalid\n";
                    }
                    break;
                }
                if (cmd.args.size() == 2) {
                    if (am.login(cmd.args[0], cmd.args[1])) {
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
                    //std::cout << "Invalid\n";
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

            default:
                std::cout << "Invalid\n";
                break;
        }
    }

    return 0;
}
