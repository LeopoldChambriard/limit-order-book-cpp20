#pragma once

#include "Order.hpp"

namespace lob {

struct LimitLevel {
    Price    price{0};
    Quantity total_volume{0};
    uint32_t order_count{0};
    
    Order* head{nullptr};
    Order* tail{nullptr};

    LimitLevel* prev{nullptr};
    LimitLevel* next{nullptr};

    explicit LimitLevel(Price p) noexcept : price(p) {}

    // Ajout d'un ordre en fin de file (FIFO) en O(1)
    void append(Order* order) noexcept {
        order->parent_level = this;
        order->next = nullptr;
        order->prev = tail;

        if (tail) {
            tail->next = order;
        } else {
            head = order;
        }
        tail = order;

        total_volume += order->quantity;
        ++order_count;
    }

    // Suppression d'un ordre en O(1) (ex. annulation ou exécution totale)
    void remove(Order* order) noexcept {
        if (order->prev) {
            order->prev->next = order->next;
        } else {
            head = order->next;
        }

        if (order->next) {
            order->next->prev = order->prev;
        } else {
            tail = order->prev;
        }

        total_volume -= order->quantity;
        --order_count;
        
        order->prev = nullptr;
        order->next = nullptr;
        order->parent_level = nullptr;
    }

    [[nodiscard]] bool empty() const noexcept {
        return order_count == 0;
    }
};

} // namespace lob