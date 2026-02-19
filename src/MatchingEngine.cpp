#include "lob/MatchingEngine.hpp"

namespace lob {

std::vector<Trade> MatchingEngine::submit_order(OrderId id, Price price, Quantity qty, Side side, OrderType type) noexcept {
    if (qty == 0) return {};

    switch (type) {
        case OrderType::Limit:
            return match_limit_order(id, price, qty, side);
        case OrderType::Market:
            return match_market_order(id, qty, side);
        default:
            return {};
    }
}

std::vector<Trade> MatchingEngine::match_limit_order(OrderId id, Price price, Quantity qty, Side side) noexcept {
    std::vector<Trade> trades;
    Quantity remaining_qty = qty;
    const Timestamp now = current_timestamp();

    if (side == Side::Buy) {
        while (remaining_qty > 0) {
            LimitLevel* best_ask = book_.best_ask_level();
            if (!best_ask || best_ask->price > price) {
                break; // Plus d'ordres passifs croisant le prix limite acheteur
            }

            Order* maker = best_ask->head;
            while (maker && remaining_qty > 0) {
                Order* next_maker = maker->next;
                Quantity traded_qty = std::min(remaining_qty, maker->quantity);

                trades.push_back(Trade{
                    .maker_order_id = maker->id,
                    .taker_order_id = id,
                    .price = maker->price,
                    .quantity = traded_qty,
                    .timestamp = now
                });

                remaining_qty -= traded_qty;
                maker->quantity -= traded_qty;

                if (maker->quantity == 0) {
                    book_.cancel_order(maker->id);
                }

                maker = next_maker;
            }
        }

        // Le volume résiduel devient passif dans le carnet
        if (remaining_qty > 0) {
            book_.add_resting_order(id, price, remaining_qty, Side::Buy);
        }
    } else { // Side::Sell
        while (remaining_qty > 0) {
            LimitLevel* best_bid = book_.best_bid_level();
            if (!best_bid || best_bid->price < price) {
                break; // Plus d'ordres passifs croisant le prix limite vendeur
            }

            Order* maker = best_bid->head;
            while (maker && remaining_qty > 0) {
                Order* next_maker = maker->next;
                Quantity traded_qty = std::min(remaining_qty, maker->quantity);

                trades.push_back(Trade{
                    .maker_order_id = maker->id,
                    .taker_order_id = id,
                    .price = maker->price,
                    .quantity = traded_qty,
                    .timestamp = now
                });

                remaining_qty -= traded_qty;
                maker->quantity -= traded_qty;

                if (maker->quantity == 0) {
                    book_.cancel_order(maker->id);
                }

                maker = next_maker;
            }
        }

        if (remaining_qty > 0) {
            book_.add_resting_order(id, price, remaining_qty, Side::Sell);
        }
    }

    return trades;
}

std::vector<Trade> MatchingEngine::match_market_order(OrderId id, Quantity qty, Side side) noexcept {
    std::vector<Trade> trades;
    Quantity remaining_qty = qty;
    const Timestamp now = current_timestamp();

    if (side == Side::Buy) {
        while (remaining_qty > 0) {
            LimitLevel* best_ask = book_.best_ask_level();
            if (!best_ask) break; // Carnet vidé

            Order* maker = best_ask->head;
            while (maker && remaining_qty > 0) {
                Order* next_maker = maker->next;
                Quantity traded_qty = std::min(remaining_qty, maker->quantity);

                trades.push_back(Trade{
                    .maker_order_id = maker->id,
                    .taker_order_id = id,
                    .price = maker->price,
                    .quantity = traded_qty,
                    .timestamp = now
                });

                remaining_qty -= traded_qty;
                maker->quantity -= traded_qty;

                if (maker->quantity == 0) {
                    book_.cancel_order(maker->id);
                }

                maker = next_maker;
            }
        }
    } else { // Side::Sell
        while (remaining_qty > 0) {
            LimitLevel* best_bid = book_.best_bid_level();
            if (!best_bid) break;

            Order* maker = best_bid->head;
            while (maker && remaining_qty > 0) {
                Order* next_maker = maker->next;
                Quantity traded_qty = std::min(remaining_qty, maker->quantity);

                trades.push_back(Trade{
                    .maker_order_id = maker->id,
                    .taker_order_id = id,
                    .price = maker->price,
                    .quantity = traded_qty,
                    .timestamp = now
                });

                remaining_qty -= traded_qty;
                maker->quantity -= traded_qty;

                if (maker->quantity == 0) {
                    book_.cancel_order(maker->id);
                }

                maker = next_maker;
            }
        }
    }

    return trades;
}

} // namespace lob