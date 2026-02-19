#pragma once

#include "Types.hpp"

namespace lob {

struct LimitLevel; // Déclaration anticipée

struct alignas(64) Order {
    OrderId     id{0};
    Price       price{0};
    Quantity    quantity{0};
    Side        side{Side::Buy};
    LimitLevel* parent_level{nullptr};
    
    // Pointeurs intrusifs pour la liste doublement chaînée (FIFO)
    Order* prev{nullptr};
    Order* next{nullptr};

    constexpr Order() = default;

    constexpr Order(OrderId id_, Price price_, Quantity qty_, Side side_) noexcept
        : id(id_), price(price_), quantity(qty_), side(side_) {}
};

} // namespace lob