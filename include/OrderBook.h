#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "Types.h"
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <string>

// Callbacks for events
#include <functional>

class OrderBook {
private:
    std::string symbol;
    
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

    void matchLimitOrder(Order& order, std::vector<Trade>& trades);
    void matchMarketOrder(Order& order, std::vector<Trade>& trades);

public:
    explicit OrderBook(std::string sym) : symbol(std::move(sym)) {}

    // Process an order and return the trades generated
    std::vector<Trade> addOrder(Order order);
    
    // Cancel an order, returning true if successful
    bool cancelOrder(const std::string& orderId);
    
    void printBook() const;
    void printBBO() const;
    
    // Check if the order can be fully filled (for FOK)
    bool canFullyFill(Side side, double price, uint64_t quantity) const;
};

#endif // ORDERBOOK_H
