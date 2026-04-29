#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "Order.h"
#include <map>
#include <list>
#include <unordered_map>
#include <string>
#include <iostream>

class OrderBook {
private:
    // Bids: Highest price first
    std::map<double, std::list<Order>, std::greater<double>> bids;
    // Asks: Lowest price first
    std::map<double, std::list<Order>, std::less<double>> asks;

    // Fast lookup for cancel and self-trade prevention
    struct OrderLocation {
        Side side;
        double price;
        std::list<Order>::iterator it;
    };
    std::unordered_map<std::string, OrderLocation> orders;

    uint64_t currentTimestamp = 0;

    void matchLimitOrder(Order& order);
    void matchMarketOrder(Order& order);

public:
    void addOrder(const std::string& orderId, Side side, double price, uint64_t quantity, OrderType type = OrderType::LIMIT);
    void cancelOrder(const std::string& orderId);
    
    void printBook() const;
    void printBBO() const;
    
    // Check if the order can be fully filled (for FOK)
    bool canFullyFill(Side side, double price, uint64_t quantity) const;
};

#endif // ORDERBOOK_H
