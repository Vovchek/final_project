#include <benchmark/benchmark.h>
#include "producer_consumer.hpp"

using namespace benchmark;

// Example usage
struct MyData {
    int priority;
    std::string value;
};


static void BM_prodcon(State& state) {
  const int n = state.range(0);

  std::vector<float> a;
  for (auto _ : state) {
    continue;
  }
  //DoNotOptimize(a);
}

BENCHMARK(BM_prodcon)->Range(1, 10);
