#include "producer_consumer.hpp"
// Example usage
struct MyData {
    int priority;
    std::string value;
};

int main() {
    // Create a queue with 2 producers and 2 consumers
    ProducerConsumerQueue<MyData, 2, 2> queue;

    // Create producer threads
    std::vector<std::thread> producerThreads;
    for (int i = 0; i < 2; ++i) {
        producerThreads.emplace_back([&queue, i]() {
            for (int j = 0; j < 5; ++j) {
                MyData data;
                data.priority = j % 3;
                data.value = "Producer " + std::to_string(i) + ", Data " + std::to_string(j);
                queue.produce(data, data.priority, -1);
                //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }

    // Create consumer threads
    std::vector<std::thread> consumerThreads;
    for (int i = 0; i < 2; ++i) {
        consumerThreads.emplace_back([&queue, i]() {
            for (int j = 0; j < 5; ++j) {
                MyData data;
                if (queue.consume(data, -1)) {
                    std::cout << "Consumer " << i << " received: " << data.value << ":" << data.priority << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        });
    }

    // Wait for all threads to finish
    for (auto& thread : producerThreads) {
        thread.join();
    }
    for (auto& thread : consumerThreads) {
        thread.join();
    }

    return 0;
}
