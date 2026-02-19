#include "lob/OrderBook.hpp"

namespace lob {

OrderBook::~OrderBook() {
    for (auto& [price, level] : bids_) {
        level_pool_.deallocate(level);
    }
    for (auto& [price, level] : asks_) {
        level_pool_.deallocate(level);
    }
}

LimitLevel* OrderBook::get_or_create_level(Side side, Price price) noexcept {
    if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it != bids_.end()) {
            return it->second;
        }
        LimitLevel* level = level_pool_.allocate();
        new (level) LimitLevel(price);
        bids_.emplace(price, level);
        return level;
    } else {
        auto it = asks_.find(price);
        if (it != asks_.end()) {
            return it->second;
        }
        LimitLevel* level = level_pool_.allocate();
        new (level) LimitLevel(price);
        asks_.emplace(price, level);
        return level;
    }
}

Order* OrderBook::add_resting_order(OrderId id, Price price, Quantity qty, Side side) noexcept {
    LimitLevel* level = get_or_create_level(side, price);
    
    Order* order = order_pool_.allocate();
    new (order) Order(id, price, qty, side);
    
    level->append(order);
    orders_map_[id] = order;
    
    return order;
}

bool OrderBook::cancel_order(OrderId id) noexcept {
    auto it = orders_map_.find(id);
    if (it == orders_map_.end()) {
        return false;
    }

    Order* order = it->second;
    LimitLevel* level = order->parent_level;

    level->remove(order);
    
    // Nettoyer le niveau s'il ne contient plus d'ordres
    if (level->empty()) {
        remove_level(order->side, level->price);
    }

    orders_map_.erase(it);
    order_pool_.deallocate(order);
    return true;
}

Order* OrderBook::find_order(OrderId id) const noexcept {
    auto it = orders_map_.find(id);
    return (it != orders_map_.end()) ? it->second : nullptr;
}

void OrderBook::remove_level(Side side, Price price) noexcept {
    if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it != bids_.end()) {
            level_pool_.deallocate(it->second);
            bids_.erase(it);
        }
    } else {
        auto it = asks_.find(price);
        if (it != asks_.end()) {
            level_pool_.deallocate(it->second);
            asks_.erase(it);
        }
    }
}

std::optional<Price> OrderBook::best_bid() const noexcept {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const noexcept {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

std::optional<Price> OrderBook::spread() const noexcept {
    auto bid = best_bid();
    auto ask = best_ask();
    if (bid && ask && *ask >= *bid) {
        return *ask - *bid;
    }
    return std::nullopt;
}

LimitLevel* OrderBook::best_bid_level() const noexcept {
    if (bids_.empty()) return nullptr;
    return bids_.begin()->second;
}

LimitLevel* OrderBook::best_ask_level() const noexcept {
    if (asks_.empty()) return nullptr;
    return asks_.begin()->second;
}

} // namespace lob