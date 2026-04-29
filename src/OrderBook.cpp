#include "OrderBook.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

vector<Trade> OrderBook::addOrder(Order order) {
    vector<Trade> trades;
    if (order.quantity == 0) return trades;

    // Reject duplicate Order IDs across the whole engine/book
    if (orders.find(order.orderId) != orders.end()) {
        cerr << "[ERROR] Rejected: Order ID " << order.orderId << " already exists.\n";
        return trades;
    }

    order.timestamp = ++currentTimestamp;

    if (order.type == OrderType::FOK) {
        if (!canFullyFill(order.side, order.price, order.quantity)) {
            // Killed immediately
            cout << "[KILLED] FOK Order " << order.orderId << " killed (no liquidity).\n";
            return trades;
        }
    }

    if (order.type == OrderType::MARKET || order.price == 0.0) {
        matchMarketOrder(order, trades);
    } else {
        matchLimitOrder(order, trades);
    }

    // Add remaining to book if not FOK/IOC and still has quantity
    if (order.quantity > 0 && order.type == OrderType::LIMIT) {
        if (order.side == Side::BUY) {
            bids[order.price].push_back(order);
            orders[order.orderId] = {order.side, order.price, --bids[order.price].end()};
        } else {
            asks[order.price].push_back(order);
            orders[order.orderId] = {order.side, order.price, --asks[order.price].end()};
        }
    } else if (order.quantity > 0 && order.type == OrderType::IOC) {
        cout << "[CANCELED] IOC Order " << order.orderId << " canceled remaining " << order.quantity << "\n";
    }

    return trades;
}

bool OrderBook::cancelOrder(const string& orderId) {
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
        return true;
    }
    return false;
}

void OrderBook::matchLimitOrder(Order& order, vector<Trade>& trades) {
    if (order.side == Side::BUY) {
        while (order.quantity > 0 && !asks.empty()) {
            auto bestAsk = asks.begin();
            if (bestAsk->first > order.price) {
                break; // No overlapping price
            }

            auto& orderList = bestAsk->second;
            auto restingOrderIt = orderList.begin();
            while (order.quantity > 0 && restingOrderIt != orderList.end()) {
                // Self-Trade Prevention (Cancel Resting)
                if (order.traderId == restingOrderIt->traderId) {
                    cout << "[STP] Self-trade prevented. Canceling resting order " << restingOrderIt->orderId << "\n";
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                    continue;
                }

                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                trades.emplace_back(restingOrderIt->orderId, order.orderId, symbol, tradePrice, tradeQuantity);

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                } else {
                    ++restingOrderIt;
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
            auto restingOrderIt = orderList.begin();
            while (order.quantity > 0 && restingOrderIt != orderList.end()) {
                // Self-Trade Prevention (Cancel Resting)
                if (order.traderId == restingOrderIt->traderId) {
                    cout << "[STP] Self-trade prevented. Canceling resting order " << restingOrderIt->orderId << "\n";
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                    continue;
                }

                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                trades.emplace_back(restingOrderIt->orderId, order.orderId, symbol, tradePrice, tradeQuantity);

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                } else {
                    ++restingOrderIt;
                }
            }

            if (orderList.empty()) {
                bids.erase(bestBid);
            }
        }
    }
}

void OrderBook::matchMarketOrder(Order& order, vector<Trade>& trades) {
    if (order.side == Side::BUY) {
        while (order.quantity > 0 && !asks.empty()) {
            auto bestAsk = asks.begin();
            auto& orderList = bestAsk->second;

            auto restingOrderIt = orderList.begin();
            while (order.quantity > 0 && restingOrderIt != orderList.end()) {
                if (order.traderId == restingOrderIt->traderId) {
                    cout << "[STP] Self-trade prevented. Canceling resting order " << restingOrderIt->orderId << "\n";
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                    continue;
                }

                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                trades.emplace_back(restingOrderIt->orderId, order.orderId, symbol, tradePrice, tradeQuantity);

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                } else {
                    ++restingOrderIt;
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

            auto restingOrderIt = orderList.begin();
            while (order.quantity > 0 && restingOrderIt != orderList.end()) {
                if (order.traderId == restingOrderIt->traderId) {
                    cout << "[STP] Self-trade prevented. Canceling resting order " << restingOrderIt->orderId << "\n";
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                    continue;
                }

                uint64_t tradeQuantity = min(order.quantity, restingOrderIt->quantity);
                double tradePrice = restingOrderIt->price;

                trades.emplace_back(restingOrderIt->orderId, order.orderId, symbol, tradePrice, tradeQuantity);

                order.quantity -= tradeQuantity;
                restingOrderIt->quantity -= tradeQuantity;

                if (restingOrderIt->quantity == 0) {
                    orders.erase(restingOrderIt->orderId);
                    restingOrderIt = orderList.erase(restingOrderIt);
                } else {
                    ++restingOrderIt;
                }
            }

            if (orderList.empty()) {
                bids.erase(bestBid);
            }
        }
    }
    if (order.quantity > 0) {
        cout << "[CANCELED] Market Order " << order.orderId << " canceled remaining " << order.quantity << " (no liquidity)\n";
    }
    order.quantity = 0; 
}

bool OrderBook::canFullyFill(Side side, double price, uint64_t quantity) const {
    uint64_t accumulatedQty = 0;
    if (side == Side::BUY) {
        for (const auto& askLevel : asks) {
            if (askLevel.first > price) break;
            for (const auto& ord : askLevel.second) {
                // Approximate STP check for FOK could be added, but skipping for simplicity
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
    cout << "--- Book for " << symbol << " ---\n";
    
    int count = 0;
    for (auto it = asks.begin(); it != asks.end() && count < 5; ++it, ++count) {
        uint64_t totalQty = 0;
        for (const auto& o : it->second) totalQty += o.quantity;
        cout << "ASK: " << fixed << setprecision(2) << it->first << " x " << totalQty << "\n";
    }
    if (asks.empty()) cout << "ASK: (empty)\n";

    count = 0;
    for (auto it = bids.begin(); it != bids.end() && count < 5; ++it, ++count) {
        uint64_t totalQty = 0;
        for (const auto& o : it->second) totalQty += o.quantity;
        cout << "BID: " << fixed << setprecision(2) << it->first << " x " << totalQty << "\n";
    }
    if (bids.empty()) cout << "BID: (empty)\n";
}

void OrderBook::printBBO() const {
    cout << "[BBO " << symbol << "] ";
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
