#include "Transaction.h"
#include <iomanip>
#include <iostream>

Transaction::Transaction(Persistence &db ,AccountManager &am):db(db), am(am)
{}

void Transaction::add(double delta, const std::string &type) {
    if (!am.isLoggedIn()) return;

    std::string userID = am.currentUserID();
    db.addFinanceRecord(delta, userID, type);
}

void Transaction::showFinance(int count) const {
    std::vector<FinanceRecord> records;
    if (count == 0) {
        std::cout << "\n";
        return;
    }

    if (!db.getFinanceRecord(records, count)) {
        std::cout << "Invalid\n";
        return;
    }

    double revenue = 0, expense = 0;
    for (const auto &r : records) {
        if (r.delta > 0) {
            revenue += r.delta;
        }
        else {
            expense -= r.delta;
        }
    }
    std::cout << std::fixed << std::setprecision(2) << "+ " << revenue << " - " << expense << "\n";
}

void Transaction::showFinanceAll() const {
    std::vector<FinanceRecord> records;

    if (!db.getFinanceRecordAll(records)) {
        std::cout << "Invalid\n";
        return;
    }

    double revenue = 0, expense = 0;
    for (const auto &r : records) {
        if (r.delta > 0) {
            revenue += r.delta;
        }
        else {
            expense -= r.delta;
        }
    }
    std::cout << std::fixed << std::setprecision(2) << "+ " << revenue << " - " << expense << "\n";
}

void Transaction::reportFinance() const {
    std::vector<FinanceRecord> records;
    if (!db.getFinanceRecordAll(records)) {
        std::cout << "Invalid\n";
        return;
    }

    double revenue = 0, expense = 0;

    std::cout << "================ Finance Report ================\n";
    std::cout << std::left
              << std::setw(15) << "User"
              << std::setw(15) << "Type"
              << std::setw(15) << "Amount"
              << "\n";
    std::cout << "------------------------------------------------\n";

    for (const auto &r : records) {
        std::cout << std::left
                  << std::setw(15) << r.userID
                  << std::setw(15) << r.type
                  << std::setw(15) << std::fixed << std::setprecision(2)
                  << r.delta
                  << "\n";

        if (r.delta > 0) revenue += r.delta;
        else expense -= r.delta;
    }

    std::cout << "------------------------------------------------\n";
    std::cout << std::fixed << std::setprecision(2)
              << "Total Revenue: +" << revenue << "\n"
              << "Total Expense: -" << expense << "\n";
    std::cout << "================================================\n";
}
