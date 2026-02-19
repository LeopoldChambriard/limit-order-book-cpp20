#include <benchmark/benchmark.h>
#include "lob/OrderBook.hpp"
#include "lob/MatchingEngine.hpp"

using namespace lob;

static void BM_OrderInsertion(benchmark::State& state) {
    OrderBook book;
    OrderId id = 1;

    for (auto _ : state) {
        book.add_resting_order(id++, 100, 10, Side::Buy);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_OrderInsertion);

static void BM_OrderCancel(benchmark::State& state) {
    OrderBook book;
    OrderId id = 1;

    for (auto _ : state) {
        state.PauseTiming();
        book.add_resting_order(id, 100, 10, Side::Buy);
        state.ResumeTiming();

        book.cancel_order(id);
        ++id;
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_OrderCancel);

static void BM_MatchingEngineCross(benchmark::State& state) {
    OrderBook book;
    MatchingEngine engine{book};
    OrderId id = 1;

    for (auto _ : state) {
        state.PauseTiming();
        engine.submit_order(id++, 100, 10, Side::Buy, OrderType::Limit);
        state.ResumeTiming();

        // Ordre agressif exécutant immédiatement
        benchmark::DoNotOptimize(
            engine.submit_order(id++, 100, 10, Side::Sell, OrderType::Limit)
        );
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MatchingEngineCross);

BENCHMARK_MAIN();