#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "OrderBook.h"
#include <unordered_map>
#include <string>

class MatchingEngine {
private:
    std::unordered_map<std::string, OrderBook> books;
    bool printBBO;

public:
    MatchingEngine(bool bbo = false) : printBBO(bbo) {}

    void processOrder(const Order& order);
    void processCancel(const std::string& orderId);
    
    void printAllBooks() const;
};

#endif // MATCHING_ENGINE_H
