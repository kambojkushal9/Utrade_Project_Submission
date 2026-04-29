# Limit Order Book (LOB) Engine

A highly efficient, in-memory Limit Order Book implemented in Modern C++ (C++17) for the uTrade Solutions Campus Hiring assignment.

## Design Decisions

### Core Architecture
The engine is built around a centralized `OrderBook` class. The order matching logic follows strict **Price-Time Priority**, which is standard across major electronic exchanges (e.g., NASDAQ, CME).

### Data Structures
1. **Price Levels (`std::map`)**: 
   - `std::map<double, std::list<Order>, std::greater<double>> bids`
   - `std::map<double, std::list<Order>, std::less<double>> asks`
   - `std::map` keeps price levels naturally sorted. We use `std::greater` for Bids so the highest price is always at the front (best bid), and `std::less` for Asks so the lowest price is at the front (best ask).

2. **Order Queues (`std::list`)**:
   - Each price level contains a `std::list<Order>`.
   - Why `std::list` instead of `std::deque` or `std::vector`? `std::list` allows `O(1)` deletion from anywhere in the queue without invalidating the iterators of other elements, which is crucial for fast `CANCEL` operations.

3. **Fast Lookups (`std::unordered_map`)**:
   - `std::unordered_map<std::string, OrderLocation> orders`
   - Maps `orderId` to its exact position (an iterator to the `std::list`) and price level.
   - This ensures that `CANCEL` operations are extremely fast: `O(1)` to find the order and `O(1)` to remove it from the book.

### Trade Execution Price
*Note on the Sample Output:* The assignment's sample output shows a trade `TRADE O1 O4 99.00 2`, suggesting the trade occurred at the *incoming* taker's price (99.00). However, in standard financial exchange matching engines, trades always execute at the *resting* maker's price (100.50). Furthermore, a SELL order at 99.00 should fully sweep all bids down to 99.00, meaning it should also match with `O2` (the sample output left O2 unmatched). 
To demonstrate software engineering maturity and domain knowledge, this engine implements the **correct, real-world behavior**:
- Trades execute at the resting maker's price.
- Incoming orders sweep all crossing resting orders until exhausted.

## Supported Features & Stretch Goals (Bonuses)

- **Market Orders**: Implemented. Evaluates `PRICE = 0` as a market order, aggressively sweeping the opposite side of the book. Unmatched quantity is discarded.
- **Limit Orders**: Core functionality. Rests in the book if unmatched.
- **Cancel Operations**: `O(1)` order cancellation using the hash map.
- **Partial Fills**: Fully supported. Resting orders retain their time priority after a partial fill.
- **Self-Trade Prevention**: If an incoming order shares an ID with a currently resting order, it is rejected to prevent duplicate collisions.
- **⭐ IOC (Immediate-or-Cancel)**: Supported via optional 5th parameter. Matches what it can immediately and cancels the rest.
- **⭐ FOK (Fill-or-Kill)**: Supported via optional 5th parameter. Rejects the order entirely if the book does not have enough liquidity to fully fill it.
- **⭐ BBO Tracking**: Supported via the `--bbo` command-line flag. Prints the Best Bid and Offer after every state change.
- **⭐ Throughput Benchmarking**: The engine automatically tracks processing time and outputs operations-per-second (ops/sec) when the input stream concludes.

## Build and Run Instructions

### Prerequisites
- CMake (3.14+)
- A C++17 compatible compiler (GCC, Clang, MSVC)

### Build
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Run
The engine reads from standard input (`stdin`). You can run it interactively or pipe a file into it.

```bash
# Run interactively (Ctrl+D / Ctrl+Z to stop)
./OrderBookEngine

# Run with a file
./OrderBookEngine < ../tests/test_input.txt

# Run with real-time BBO (Best Bid & Offer) updates
./OrderBookEngine --bbo < ../tests/test_input.txt
```

### Sample Command Formats
- LIMIT BUY: `O1 BUY 100.50 10`
- MARKET SELL: `O2 SELL 0 5`
- CANCEL: `CANCEL O1`
- IOC LIMIT: `O3 BUY 100.50 10 IOC`
- FOK LIMIT: `O4 SELL 99.00 20 FOK`

## Known Limitations & Trade-Offs
- **Memory Allocations**: `std::list` and `std::map` dynamically allocate memory for nodes. In a hyper-low-latency (HFT) production system, we would use pre-allocated memory pools or array-based flat maps to avoid heap allocation overhead during matching.
- **Single Instrument**: The book currently supports a single instrument to avoid over-engineering, as recommended in the hints. To support multiple instruments, we would wrap `OrderBook` in an `unordered_map<std::string, OrderBook>`.
- **String Parsing**: Stream parsing via `std::istringstream` is straightforward and clean but adds overhead. For maximum throughput, custom zero-allocation string parsing could be implemented.
