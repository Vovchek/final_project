#include <benchmark/benchmark.h>
#include <cassert>
#include <chrono>
#include <thread>
#include "producer_consumer.hpp"

using namespace benchmark;

// Example usage
struct MyData {
  int priority;
  std::string value;
};

void load(size_t n) {
  static volatile size_t x = 0;
  for(size_t i = 0; i < n; ++i) {
    x = x + 1;
  }
}

static void BM_load(State& state) {
  const int n = state.range(0);
  for (auto _ : state) {
    load(n);
  }
}

BENCHMARK(BM_load)->Range(1<<10, 1<<10);


static std::vector<std::thread> start_producers(
    ProducerConsumerQueue<MyData>& queue,
    size_t num_mes,
    size_t num_prod,
    size_t cpu_burn = 1024,
    size_t num_prio = 1) {
  assert(num_mes >= num_prod);
  assert(num_prio > 0);
  std::vector<std::thread> producerThreads;
  size_t per_prod = num_mes / num_prod;
  if (num_mes % num_prod)
    ++per_prod;
  for (size_t i_prod = 0, j_start = 0; i_prod < num_prod;
       ++i_prod, j_start += per_prod) {
    auto j_end = std::min(num_mes, j_start + per_prod);
    producerThreads.emplace_back([=, &queue]() {
      for (auto j = j_start; j < j_end; ++j) {
        MyData data;
        data.priority = j % num_prio;
        data.value = "Producer " + std::to_string(i_prod) + ", Data " +
                     std::to_string(j);
        load(cpu_burn);  // do the job
        queue.produce(data, data.priority, -1);
      }
    });
  }
  return producerThreads;
}

std::vector<std::thread> start_consumers(ProducerConsumerQueue<MyData>& queue,
                                         size_t num_cons, size_t cpu_burn = 1024) {
  // Create consumer threads
  std::vector<std::thread> consumerThreads;
  for (int i = 0; i < num_cons; ++i) {
    consumerThreads.emplace_back([&queue, num_cons, cpu_burn]() {
      for (int j = 0;; ++j) {
        MyData data;
        if (queue.consume(data, 0)) { // return false immediately on empty queue
          load(cpu_burn);  // digest the food
        } else
          break;  // no more data
      }
    });
  }
  return consumerThreads;
}

static void BM_produce(State& state) {
  const int n_prod = state.range(0);
  std::vector<std::thread> producerThreads;
  for (auto _ : state) {
    ProducerConsumerQueue<MyData> queue;
    producerThreads = start_producers(queue, 10000, n_prod);
    for (auto& thread : producerThreads) {
      thread.join();
    }
    DoNotOptimize(queue);
  }
}

BENCHMARK(BM_produce)->Range(1, std::thread::hardware_concurrency() * 2);

static void BM_consume(State& state) {
  const int n_cons = state.range(0);
  std::vector<std::thread> producerThreads, consumerThreads;
  for (auto _ : state) {
    ProducerConsumerQueue<MyData> queue;
    producerThreads = start_producers(queue, 100, 1, 1);
    consumerThreads = start_consumers(queue, n_cons);
    for (auto& thread : producerThreads) {
      thread.join();
    }
    for (auto& thread : consumerThreads) {
      thread.join();
    }
    DoNotOptimize(queue);
  }
}

BENCHMARK(BM_consume)->Range(1, std::thread::hardware_concurrency() * 2);

