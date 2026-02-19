#include <gtest/gtest.h>
#include "lob/OrderBook.hpp"
#include "lob/MatchingEngine.hpp"

using namespace lob;

class MatchingEngineTest : public ::testing::Test {
protected:
    OrderBook book;
    MatchingEngine engine{book};
};

TEST_F(MatchingEngineTest, AddRestingOrdersAndVerifyBBO) {
    engine.submit_order(1, 100, 10, Side::Buy, OrderType::Limit);
    engine.submit_order(2, 105, 10, Side::Sell, OrderType::Limit);

    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_EQ(book.best_ask(), 105);
    EXPECT_EQ(book.spread(), 5);
}

TEST_F(MatchingEngineTest, FullCrossExecution) {
    engine.submit_order(1, 100, 10, Side::Buy, OrderType::Limit);
    auto trades = engine.submit_order(2, 100, 10, Side::Sell, OrderType::Limit);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].maker_order_id, 1);
    EXPECT_EQ(trades[0].taker_order_id, 2);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 10);

    EXPECT_EQ(book.order_count(), 0);
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST_F(MatchingEngineTest, PartialExecutionAndRestingRemainder) {
    engine.submit_order(1, 100, 10, Side::Buy, OrderType::Limit);
    auto trades = engine.submit_order(2, 100, 4, Side::Sell, OrderType::Limit);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 4);

    EXPECT_EQ(book.order_count(), 1);
    EXPECT_EQ(book.best_bid(), 100);
    
    Order* maker = book.find_order(1);
    ASSERT_NE(maker, nullptr);
    EXPECT_EQ(maker->quantity, 6);
}

TEST_F(MatchingEngineTest, FIFOPriorityMatching) {
    // Ordre 1 et Ordre 2 insérés au même prix de 100
    engine.submit_order(1, 100, 10, Side::Buy, OrderType::Limit);
    engine.submit_order(2, 100, 10, Side::Buy, OrderType::Limit);

    // Ordre vendeur agressif de 15 titres : doit consommer entièrement l'ordre 1, puis 5 de l'ordre 2
    auto trades = engine.submit_order(3, 100, 15, Side::Sell, OrderType::Limit);

    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].maker_order_id, 1);
    EXPECT_EQ(trades[0].quantity, 10);
    EXPECT_EQ(trades[1].maker_order_id, 2);
    EXPECT_EQ(trades[1].quantity, 5);

    EXPECT_EQ(book.find_order(1), nullptr);
    Order* remaining = book.find_order(2);
    ASSERT_NE(remaining, nullptr);
    EXPECT_EQ(remaining->quantity, 5);
}

TEST_F(MatchingEngineTest, CancelOrder) {
    engine.submit_order(1, 100, 10, Side::Buy, OrderType::Limit);
    EXPECT_EQ(book.order_count(), 1);

    EXPECT_TRUE(engine.cancel_order(1));
    EXPECT_EQ(book.order_count(), 0);
    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_FALSE(engine.cancel_order(1)); // Deuxième annulation échoue proprement
}