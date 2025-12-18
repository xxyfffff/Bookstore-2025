#include "Transaction.h"
#include <iomanip>
#include <iostream>

Transaction::Transaction(Persistence &db):db(db)
{}

void Transaction::add(double delta) {
    db.addFinanceRecord(delta);
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