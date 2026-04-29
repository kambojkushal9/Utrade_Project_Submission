#include "OrderBook.h"
#include <iostream>
#include <iomanip>

using namespace std;

void OrderBook::addOrder(const string& orderId, Side side, double price, uint64_t quantity, OrderType type) {
    if (quantity == 0) return;

    // Self-trade prevention (Reject duplicate Order IDs)
    if (orders.find(orderId) != orders.end()) {
        cerr << "Rejected: Order ID " << orderId << " already exists.\n";
        return;
    }

    Order newOrder(orderId, side, price, quantity, type, ++currentTimestamp);

    if (type == OrderType::FOK) {
        if (!canFullyFill(side, price, quantity)) {
            // Killed immediately
            return;
        }
    }

    if (type == OrderType::MARKET || price == 0) {
        matchMarketOrder(newOrder);
    } else {
        matchLimitOrder(newOrder);
    }

    // Add remaining to book if not FOK/IOC and still has quantity
    if (newOrder.quantity > 0 && newOrder.type == OrderType::LIMIT) {
        if (newOrder.side == Side::BUY) {
            bids[newOrder.price].push_back(newOrder);
            orders[newOrder.orderId] = {newOrder.side, newOrder.price, --bids[newOrder.price].end()};
        } else {
            asks[newOrder.price].push_back(newOrder);
            orders[newOrder.orderId] = {newOrder.side, newOrder.price, --asks[newOrder.price].end()};
        }
    }
}

void OrderBook::cancelOrder(const string& orderId) {
    auto it = orders.find(orderId);
    if (it != orders.end()) {
        Side side = it->second.side;
        double price = it->second.price;
        auto listIt = it->second.it;

        if (side == Side::BUY) {
            bids[price].erase(listIt);
            if (bids[price].empty()) {
                bids.erase(price);
            }
        } else {
            asks[price].erase(listIt);
            if (asks[price].empty()) {
                asks.erase(price);
            }
        }
        orders.erase(it);
    }
}

void OrderBook::matchLimitOrder(Order& order) {
    if (order.side == Side::BUY) {
        while (order.quantity > 0 && !asks.empty()) {
            auto bestAsk = asks.begin();
            if (bestAsk->first > order.price) {
                break; // No overlapping price
            }

            auto& orderList = bestAsk->second;
            while (order.quantity > 0 && !orderList.empty()) {
                auto restingOrderIt = orderList.begin();
                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                cout << "TRADE " << order.orderId << " " << restingOrderIt->orderId 
                     << " " << fixed << setprecision(2) << tradePrice << " " << tradeQuantity << "\n";

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    orderList.pop_front();
                }
            }

            if (orderList.empty()) {
                asks.erase(bestAsk);
            }
        }
    } else { // SELL
        while (order.quantity > 0 && !bids.empty()) {
            auto bestBid = bids.begin();
            if (bestBid->first < order.price) {
                break; // No overlapping price
            }

            auto& orderList = bestBid->second;
            while (order.quantity > 0 && !orderList.empty()) {
                auto restingOrderIt = orderList.begin();
                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                cout << "TRADE " << restingOrderIt->orderId << " " << order.orderId 
                     << " " << fixed << setprecision(2) << tradePrice << " " << tradeQuantity << "\n";

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    orderList.pop_front();
                }
            }

            if (orderList.empty()) {
                bids.erase(bestBid);
            }
        }
    }
}

void OrderBook::matchMarketOrder(Order& order) {
    if (order.side == Side::BUY) {
        while (order.quantity > 0 && !asks.empty()) {
            auto bestAsk = asks.begin();
            auto& orderList = bestAsk->second;

            while (order.quantity > 0 && !orderList.empty()) {
                auto restingOrderIt = orderList.begin();
                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                cout << "TRADE " << order.orderId << " " << restingOrderIt->orderId 
                     << " " << fixed << setprecision(2) << tradePrice << " " << tradeQuantity << "\n";

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    orderList.pop_front();
                }
            }

            if (orderList.empty()) {
                asks.erase(bestAsk);
            }
        }
    } else { // SELL
        while (order.quantity > 0 && !bids.empty()) {
            auto bestBid = bids.begin();
            auto& orderList = bestBid->second;

            while (order.quantity > 0 && !orderList.empty()) {
                auto restingOrderIt = orderList.begin();
                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                cout << "TRADE " << restingOrderIt->orderId << " " << order.orderId 
                     << " " << fixed << setprecision(2) << tradePrice << " " << tradeQuantity << "\n";

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    orderList.pop_front();
                }
            }

            if (orderList.empty()) {
                bids.erase(bestBid);
            }
        }
    }
    // Market orders do not rest in the book. Remaining quantity is discarded.
    order.quantity = 0; 
}

bool OrderBook::canFullyFill(Side side, double price, uint64_t quantity) const {
    uint64_t accumulatedQty = 0;
    if (side == Side::BUY) {
        for (const auto& askLevel : asks) {
            if (askLevel.first > price) break;
            for (const auto& ord : askLevel.second) {
                accumulatedQty += ord.quantity;
                if (accumulatedQty >= quantity) return true;
            }
        }
    } else {
        for (const auto& bidLevel : bids) {
            if (bidLevel.first < price) break;
            for (const auto& ord : bidLevel.second) {
                accumulatedQty += ord.quantity;
                if (accumulatedQty >= quantity) return true;
            }
        }
    }
    return false;
}

void OrderBook::printBook() const {
    cout << "--- Book ---\n";
    
    // Asks (Print top 5 lowest)
    int count = 0;
    for (auto it = asks.begin(); it != asks.end() && count < 5; ++it, ++count) {
        uint64_t totalQty = 0;
        for (const auto& o : it->second) totalQty += o.quantity;
        cout << "ASK: " << fixed << setprecision(2) << it->first << " x " << totalQty << "\n";
    }
    if (asks.empty()) cout << "ASK: (empty)\n";

    // Bids (Print top 5 highest)
    count = 0;
    for (auto it = bids.begin(); it != bids.end() && count < 5; ++it, ++count) {
        uint64_t totalQty = 0;
        for (const auto& o : it->second) totalQty += o.quantity;
        cout << "BID: " << fixed << setprecision(2) << it->first << " x " << totalQty << "\n";
    }
    if (bids.empty()) cout << "BID: (empty)\n";
}

void OrderBook::printBBO() const {
    cout << "[BBO] ";
    if (!bids.empty()) {
        uint64_t totalQty = 0;
        for (const auto& o : bids.begin()->second) totalQty += o.quantity;
        cout << "BID: " << fixed << setprecision(2) << bids.begin()->first << " (" << totalQty << ") ";
    } else {
        cout << "BID: NONE ";
    }

    if (!asks.empty()) {
        uint64_t totalQty = 0;
        for (const auto& o : asks.begin()->second) totalQty += o.quantity;
        cout << "| ASK: " << fixed << setprecision(2) << asks.begin()->first << " (" << totalQty << ")\n";
    } else {
        cout << "| ASK: NONE\n";
    }
}
