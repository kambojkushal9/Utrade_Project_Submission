#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <cstdint>

enum class Side {
    BUY,
    SELL
};

enum class OrderType {
    LIMIT,
    MARKET,
    IOC, // Immediate or Cancel
    FOK  // Fill or Kill
};

struct Order {
    std::string orderId;
    std::string symbol;
    Side side;
    double price;       // 0 for MARKET orders
    uint64_t quantity;
    std::string traderId;
    OrderType type;
    uint64_t timestamp; 

    Order(std::string id, std::string sym, Side s, double p, uint64_t q, std::string tId, OrderType t = OrderType::LIMIT, uint64_t ts = 0)
        : orderId(std::move(id)), symbol(std::move(sym)), side(s), price(p), quantity(q), traderId(std::move(tId)), type(t), timestamp(ts) {}
};

struct Trade {
    std::string makerOrderId;
    std::string takerOrderId;
    std::string symbol;
    double price;
    uint64_t quantity;

    Trade(std::string makerId, std::string takerId, std::string sym, double p, uint64_t q)
        : makerOrderId(std::move(makerId)), takerOrderId(std::move(takerId)), symbol(std::move(sym)), price(p), quantity(q) {}
};

#endif // TYPES_H
