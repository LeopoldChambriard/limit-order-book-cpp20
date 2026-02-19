#pragma once

#include <cstdint>
#include <string_view>

namespace lob {

using OrderId   = uint64_t;
using Price     = uint32_t;  // Prix en ticks entiers pour éviter les erreurs d'arrondi flottant
using Quantity  = uint32_t;
using Timestamp = uint64_t;  // Nanosecondes UNIX

enum class Side : uint8_t {
    Buy  = 0,
    Sell = 1
};

enum class OrderType : uint8_t {
    Limit  = 0,
    Market = 1,
    Cancel = 2
};

struct Trade {
    OrderId   maker_order_id;
    OrderId   taker_order_id;
    Price     price;
    Quantity  quantity;
    Timestamp timestamp;
};

} // namespace lob