#pragma once

#include "OrderBook.hpp"
#include <vector>
#include <chrono>

namespace lob {

class MatchingEngine {
public:
    explicit MatchingEngine(OrderBook& book) noexcept : book_(book) {}

    // Traite un ordre entrant (Limit ou Market) et retourne les transactions exécutées
    std::vector<Trade> submit_order(OrderId id, Price price, Quantity qty, Side side, OrderType type) noexcept;

    // Annule un ordre existant
    bool cancel_order(OrderId id) noexcept {
        return book_.cancel_order(id);
    }

private:
    std::vector<Trade> match_limit_order(OrderId id, Price price, Quantity qty, Side side) noexcept;
    std::vector<Trade> match_market_order(OrderId id, Quantity qty, Side side) noexcept;

    [[nodiscard]] static Timestamp current_timestamp() noexcept {
        return static_cast<Timestamp>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count()
        );
    }

    OrderBook& book_;
};

} // namespace lob