#pragma once

#include "Types.hpp"
#include "Order.hpp"
#include "LimitLevel.hpp"
#include "MemoryPool.hpp"

#include <map>
#include <unordered_map>
#include <optional>
#include <functional>

namespace lob {

class OrderBook {
public:
    OrderBook() = default;
    ~OrderBook();

    // Empêcher les copies pour éviter les allocations accidentelles
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook(OrderBook&&) noexcept = default;
    OrderBook& operator=(OrderBook&&) noexcept = default;

    // Insertion d'un ordre passif dans le carnet
    Order* add_resting_order(OrderId id, Price price, Quantity qty, Side side) noexcept;

    // Annulation immédiate en O(1) via la table de pointeurs
    bool cancel_order(OrderId id) noexcept;

    // Récupération d'un ordre par identifiant
    [[nodiscard]] Order* find_order(OrderId id) const noexcept;

    // Consultation des meilleurs prix (BBO : Best Bid / Offer)
    [[nodiscard]] std::optional<Price> best_bid() const noexcept;
    [[nodiscard]] std::optional<Price> best_ask() const noexcept;
    [[nodiscard]] std::optional<Price> spread() const noexcept;

    // Accès aux niveaux de prix pour le moteur d'appariement
    [[nodiscard]] LimitLevel* best_bid_level() const noexcept;
    [[nodiscard]] LimitLevel* best_ask_level() const noexcept;

    // Suppression d'un niveau de prix vidé
    void remove_level(Side side, Price price) noexcept;

    // Métriques de profondeur
    [[nodiscard]] size_t order_count() const noexcept { return orders_map_.size(); }

private:
    LimitLevel* get_or_create_level(Side side, Price price) noexcept;

    // Niveaux de prix triés (Bids: max-first, Asks: min-first)
    std::map<Price, LimitLevel*, std::greater<Price>> bids_;
    std::map<Price, LimitLevel*, std::less<Price>>    asks_;

    // Indexation O(1) des ordres par ID
    std::unordered_map<OrderId, Order*> orders_map_;

    // Pools de mémoire contigus (zéro-allocation système sur le chemin critique)
    MemoryPool<Order, 100'000>      order_pool_;
    MemoryPool<LimitLevel, 10'000>  level_pool_;
};

} // namespace lob