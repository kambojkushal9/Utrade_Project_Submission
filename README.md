# High-Frequency Limit Order Book (LOB) Engine

A highly efficient, strictly-prioritized in-memory Limit Order Book implemented in Modern C++ (C++17) for the uTrade Solutions Campus Hiring assignment.

Designed with architecture suitable for high-throughput environments, this implementation showcases advanced C++ concepts, strict adherence to SOLID principles, and real-world trading edge cases like Self-Trade Prevention (STP) and Multi-Instrument support.

---

## 🚀 Key Features (Top 1% Standards)

### 1. Multi-Instrument Matching Engine
Unlike basic implementations that hardcode a single asset, this engine introduces a `MatchingEngine` router layer. It dynamically instantiates and manages multiple `OrderBook` objects (`unordered_map<string, OrderBook>`), allowing seamless trading across AAPL, GOOG, TSLA, and more simultaneously.

### 2. Self-Trade Prevention (STP)
Real-world exchanges must prevent Wash Trading. This engine includes a strict **Cancel Resting (STP-CR)** policy. If an incoming order shares the same `TraderID` as a resting crossing order, the resting order is automatically canceled, and the incoming order continues sweeping the book.

### 3. Decoupled Architecture (SOLID)
The initial assignment code often mixes I/O with core business logic (e.g., `std::cout` inside `matchOrder`). 
- This engine is refactored into distinct components. The `OrderBook` strictly processes logic and returns `std::vector<Trade>`.
- The `MatchingEngine` and `main.cpp` handle the I/O. This makes unit testing trivial and allows easy migration to a network-based TCP/UDP socket interface.

### 4. Zero-Allocation String Parsing
String parsing is often the biggest bottleneck in CLI trading engines. By utilizing `std::string_view` for zero-allocation tokenization, the engine bypasses heavy memory allocations during standard `std::istringstream` processing, vastly increasing the `ops/sec` throughput.

### 5. Advanced Order Types
- **Market Orders**: Aggressively sweeps liquidity at `PRICE = 0`. Discards unfilled quantity.
- **Limit Orders**: Rests in the book using strict Price-Time Priority.
- **IOC (Immediate-or-Cancel)**: Fills whatever it can immediately and cancels the rest.
- **FOK (Fill-or-Kill)**: Analyzes book depth first in `O(P)` time. Rejects the order entirely if full liquidity isn't available.

---

## 🧠 Design & Data Structures

1. **Price Levels (`std::map`)**: 
   - `std::map<double, std::list<Order>, std::greater<double>> bids`
   - Maps naturally sort price levels. Bids use `std::greater` to keep the best bid at the front. Asks use `std::less` for the lowest ask.

2. **Order Queues (`std::list`)**:
   - Why `std::list` instead of `std::deque`? `std::list` allows `O(1)` deletion without invalidating iterators, which is mandatory for ultra-fast `CANCEL` operations.

3. **Fast Lookups (`std::unordered_map`)**:
   - `std::unordered_map<std::string, OrderLocation> orders`
   - Maps `OrderID` to its exact iterator in the `std::list`.
   - Allows `CANCEL` operations to be effectively `O(1)`.

### Time Complexity
| Operation | Complexity | Explanation |
|-----------|------------|-------------|
| **Add Limit** | O(log P) | P = active price levels. Locating the price in the map takes logarithmic time. |
| **Add Market**| O(M) | M = resting orders matched. Sweeping the book is linear to the fills generated. |
| **Cancel** | O(1) | O(1) hash map lookup directly giving the `std::list` iterator for O(1) deletion. |

### Space Complexity
`O(N + P)` where N is total resting orders and P is total price levels.

---

## 🛠️ Build and Run Instructions

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
*(The `CMakeLists.txt` applies aggressive `-O3`, `-flto`, and `-march=native` compiler flags for max throughput).*

### Input Format
Due to the addition of STP and Multi-Instrument support, the input format takes 6 parameters:
`[OrderID] [Symbol] [BUY/SELL] [Price] [Qty] [TraderID] [Optional: Type]`

### Sample Command Formats
- LIMIT BUY: `O1 AAPL BUY 100.50 10 T1`
- MARKET SELL: `O2 AAPL SELL 0 5 T2`
- CANCEL: `CANCEL O1`
- IOC LIMIT: `O3 AAPL BUY 100.50 10 T3 IOC`
- FOK LIMIT: `O4 GOOG SELL 99.00 20 T4 FOK`

### Running Tests
The engine reads from `stdin` and supports a `--bbo` flag for Real-Time Best Bid Offer tracking.

```bash
# Run basic matching test
./OrderBookEngine < ../tests/test_input.txt

# Run Self-Trade Prevention (STP) test
./OrderBookEngine < ../tests/test_stp.txt

# Run Multi-Instrument test
./OrderBookEngine --bbo < ../tests/test_multi_symbol.txt
```

---

## 📉 Known Limitations (HFT Trade-Offs)
- **Heap Allocations**: `std::list` and `std::map` dynamically allocate memory. In a sub-10-microsecond HFT production system, we would replace these with pre-allocated array-based flat-maps or memory pools (e.g., `Boost::pool`) to eliminate OS heap overhead.
- **Multithreading**: This implementation runs sequentially for deterministic results. A production Matching Engine typically employs lock-free ring buffers (like the Disruptor pattern) for core-pinned thread messaging.
