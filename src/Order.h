#ifndef ORDER_H
#define ORDER_H

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
    Side side;
    double price;       // 0 for MARKET orders
    uint64_t quantity;
    OrderType type;
    uint64_t timestamp; // For strict price-time priority if needed, though std::list preserves insertion order

    Order(std::string id, Side s, double p, uint64_t q, OrderType t = OrderType::LIMIT, uint64_t ts = 0)
        : orderId(std::move(id)), side(s), price(p), quantity(q), type(t), timestamp(ts) {}
};

#endif // ORDER_H
