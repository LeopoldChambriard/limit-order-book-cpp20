# Ultra-Fast Limit Order Book & Matching Engine (C++20)

[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Standard](https://img.shields.io/badge/Standard-HFT%20Architecture-green.svg)](#)
[![Tests](https://img.shields.io/badge/Tests-GoogleTest%20Passed-brightgreen.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-black.svg)](LICENSE)

A deterministic, ultra-low-latency Limit Order Book (LOB) and Matching Engine implemented in modern C++20. 

Engineered for algorithmic trading and market-making infrastructure, the system enforces strict **Price-Time Priority (FIFO)** execution with a **zero-allocation hot path**, cache-aligned data structures, and $\mathcal{O}(1)$ order cancellations.

---

## 1. Key Architectural Design

* **Zero-Allocation on Hot Path:** Custom contiguous `MemoryPool` eliminates kernel heap allocations (`malloc`/`new`) during live trading loops. Node allocations and deallocations execute in $\mathcal{O}(1)$ without heap fragmentation.
* **Intrusive Double-Linked Lists:** Orders within each price level form an intrusive doubly linked list, enabling $\mathcal{O}(1)$ insertion at tail and $\mathcal{O}(1)$ direct node removal.
* **Constant Time Lookups:** Direct order indexation via `std::unordered_map<OrderId, Order*>` facilitates immediate $\mathcal{O}(1)$ order cancellation and modification without book traversal.
* **Integer Tick Pricing:** Prices and quantities are represented as fixed-point discrete integers (`uint32_t`), eliminating floating-point rounding errors and CPU branch mispredictions.
* **Deterministic Matching:** Strict price-time priority for both limit and aggressive market orders with complete tracking of partial fills and trade execution events.

---

## 2. Market Microstructure Mechanics

### Best Bid / Offer (BBO) and Spread

The book maintains continuous top-of-book quotes:

$$\text{Spread} = P_{\text{ask}}^* - P_{\text{bid}}^*$$

where:

$$P_{\text{ask}}^* = \min_{i} P_{\text{ask}, i}, \quad P_{\text{bid}}^* = \max_{j} P_{\text{bid}, j}$$

### Crossing Condition

An incoming aggressive order $(P_{\text{taker}}, Q_{\text{taker}}, \text{Side})$ matches resting liquidity if and only if:

$$\begin{cases}  P_{\text{taker}} \ge P_{\text{ask}}^* & \text{if } \text{Side} = \text{Buy} \\ P_{\text{taker}} \le P_{\text{bid}}^* & \text{if } \text{Side} = \text{Sell} \end{cases}$$

Trades are executed against passive makers at the maker's resting price $P_{\text{maker}}$ until $Q_{\text{taker}} = 0$ or the spread uncrosses. Unfilled limit volume converts to passive depth.

---

## 3. Directory Layout

```text
limit-order-book-cpp20/
├── CMakeLists.txt
├── README.md
├── include/
│   └── lob/
│       ├── Types.hpp          # Primitive scalar aliases, Enums & Trade structures
│       ├── Order.hpp          # Cache-aligned (64B) order node
│       ├── LimitLevel.hpp     # Price queue with intrusive FIFO list
│       ├── MemoryPool.hpp     # Pre-allocated O(1) contiguous block pool
│       ├── OrderBook.hpp      # Bids/Asks book depth & BBO tracking
│       └── MatchingEngine.hpp # Order crossing & execution dispatch
├── src/
│   ├── OrderBook.cpp
│   └── MatchingEngine.cpp
├── tests/
│   └── test_matching.cpp     # Unit tests via GoogleTest
└── benchmarks/
    └── bench_lob.cpp         # Nanosecond latency microbenchmarks