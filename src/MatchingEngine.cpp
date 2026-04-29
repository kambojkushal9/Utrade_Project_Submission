#include "MatchingEngine.h"
#include <iostream>
#include <iomanip>

using namespace std;

void MatchingEngine::processOrder(const Order& order) {
    if (books.find(order.symbol) == books.end()) {
        books.emplace(order.symbol, OrderBook(order.symbol));
    }
    
    OrderBook& book = books.at(order.symbol);
    
    cout << "[NEW] Order " << order.orderId << " accepted on " << order.symbol << "\n";
    
    vector<Trade> trades = book.addOrder(order);
    
    for (const auto& trade : trades) {
        cout << "[TRADE] " << trade.makerOrderId << " " << trade.takerOrderId 
             << " " << trade.symbol << " " << fixed << setprecision(2) << trade.price 
             << " " << trade.quantity << "\n";
    }

    if (printBBO) book.printBBO();
}

void MatchingEngine::processCancel(const string& orderId) {
    bool canceled = false;
    for (auto& [symbol, book] : books) {
        if (book.cancelOrder(orderId)) {
            cout << "[CANCELED] Order " << orderId << " canceled on " << symbol << "\n";
            if (printBBO) book.printBBO();
            canceled = true;
            break;
        }
    }
    
    if (!canceled) {
        cout << "[ERROR] Cancel failed. Order " << orderId << " not found.\n";
    }
}

void MatchingEngine::printAllBooks() const {
    if (books.empty()) {
        cout << "--- No Books Active ---\n";
        return;
    }
    for (const auto& [symbol, book] : books) {
        book.printBook();
    }
}
