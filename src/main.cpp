#include "MatchingEngine.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

// Fast string parsing utility (zero-allocation for tokens)
vector<string_view> split(string_view s, char delim) {
    vector<string_view> tokens;
    size_t start = 0;
    size_t end = s.find(delim);
    while (end != string_view::npos) {
        if (end != start) {
            tokens.push_back(s.substr(start, end - start));
        }
        start = end + 1;
        end = s.find(delim, start);
    }
    if (start < s.length()) {
        tokens.push_back(s.substr(start));
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    bool printBBO = false;

    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--bbo") {
            printBBO = true;
        }
    }

    MatchingEngine engine(printBBO);
    string line;
    uint64_t processedOrders = 0;

    auto startTime = chrono::high_resolution_clock::now();

    while (getline(cin, line)) {
        if (line.empty()) continue;

        auto tokens = split(line, ' ');
        if (tokens.empty()) continue;

        string_view cmdOrId = tokens[0];

        if (cmdOrId == "CANCEL") {
            if (tokens.size() < 2) continue;
            string orderId(tokens[1]);
            engine.processCancel(orderId);
        } else {
            // New format: [OrderID] [Symbol] [BUY/SELL] [Price] [Qty] [TraderID] [Optional: Type]
            if (tokens.size() < 6) {
                cerr << "[ERROR] Invalid format: " << line << "\n";
                continue;
            }

            string orderId(tokens[0]);
            string symbol(tokens[1]);
            string_view sideStr = tokens[2];
            double price = stod(string(tokens[3]));
            uint64_t quantity = stoull(string(tokens[4]));
            string traderId(tokens[5]);
            
            string_view typeStr = (tokens.size() > 6) ? tokens[6] : "";

            Side side = (sideStr == "BUY") ? Side::BUY : Side::SELL;
            OrderType type = OrderType::LIMIT;

            if (price == 0.0) {
                type = OrderType::MARKET;
            } else if (typeStr == "IOC") {
                type = OrderType::IOC;
            } else if (typeStr == "FOK") {
                type = OrderType::FOK;
            }

            engine.processOrder(Order(orderId, symbol, side, price, quantity, traderId, type));
        }
        processedOrders++;
    }

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = endTime - startTime;

    cout << "\n";
    engine.printAllBooks();

    cout << "\n--- Benchmarks ---\n";
    cout << "Processed " << processedOrders << " instructions in " << diff.count() << " seconds.\n";
    if (diff.count() > 0) {
        cout << "Throughput: " << fixed << setprecision(2) << (processedOrders / diff.count()) << " ops/sec\n";
    }

    return 0;
}
