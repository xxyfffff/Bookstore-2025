//
// Created by yifei on 12/12/2025.
//

#ifndef BOOKSTORE_UTILS_H
#define BOOKSTORE_UTILS_H
#include <string>
#include <vector>
#include "Types.h"

class Utils {
public:
    static std::string trim(const std::string &s);
    static bool isValidISBN(const std::string &s);
    static bool isValidKeywordChar(char c);
    static std::vector<std::string> splitKeywords(const std::string &s);

    static std::string formatPrice(double x);
};

#endif //BOOKSTORE_UTILS_H