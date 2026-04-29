#include "OrderBook.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

int main(int argc, char* argv[]) {
    OrderBook engine;
    bool printBBO = false;

    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--bbo") {
            printBBO = true;
        }
    }

    string line;
    uint64_t processedOrders = 0;

    auto startTime = chrono::high_resolution_clock::now();

    while (getline(cin, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string cmdOrId;
        iss >> cmdOrId;

        if (cmdOrId == "CANCEL") {
            string orderId;
            iss >> orderId;
            engine.cancelOrder(orderId);
            if (printBBO) engine.printBBO();
        } else {
            // It's a new order
            string orderId = cmdOrId;
            string sideStr;
            double price;
            uint64_t quantity;
            string typeStr = "";

            if (!(iss >> sideStr >> price >> quantity)) {
                cerr << "Error parsing line: " << line << "\n";
                continue;
            }

            iss >> typeStr; // Optional 5th parameter

            Side side = (sideStr == "BUY") ? Side::BUY : Side::SELL;
            OrderType type = OrderType::LIMIT;

            if (price == 0.0) {
                type = OrderType::MARKET;
            } else if (typeStr == "IOC") {
                type = OrderType::IOC;
            } else if (typeStr == "FOK") {
                type = OrderType::FOK;
            }

            engine.addOrder(orderId, side, price, quantity, type);
            if (printBBO) engine.printBBO();
        }
        processedOrders++;
    }

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = endTime - startTime;

    engine.printBook();

    cout << "\n--- Benchmarks ---\n";
    cout << "Processed " << processedOrders << " instructions in " << diff.count() << " seconds.\n";
    if (diff.count() > 0) {
        cout << "Throughput: " << fixed << setprecision(2) << (processedOrders / diff.count()) << " ops/sec\n";
    }

    return 0;
}
