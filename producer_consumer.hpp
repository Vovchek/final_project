#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

template <typename T, int MaxQueSize = 0>
class ProducerConsumerQueue {
private:
    std::queue<T> dataQueue;
    std::mutex mutex;
    std::condition_variable cvProducer;
    std::condition_variable cvConsumer;

public:
    bool produce(const T& data, int priority = 0, int timeout = 0) {
        std::unique_lock<std::mutex> lock(mutex);

        // Wait for space in the queue if it's full
        if (MaxQueSize &&dataQueue.size() >= MaxQueSize) {
            if (timeout >= 0) {
                if (cvProducer.wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
                    // Timeout occurred
                    std::cerr << "Producer timed out." << std::endl;
                    return false;
                }
            } else {
                cvProducer.wait(lock);
            }
        }

        // Insert the data into the queue based on priority
        if (priority == 0 || dataQueue.empty()) {
            dataQueue.push(data);
        } else {
            std::queue<T> tempQueue;
            bool inserted = false;

            while (!dataQueue.empty()) {
                if (dataQueue.front().priority <= priority && !inserted) {
                    tempQueue.push(data);
                    inserted = true;
                }
                tempQueue.push(dataQueue.front());
                dataQueue.pop();
            }

            if (!inserted) {
                tempQueue.push(data);
            }

            std::swap(dataQueue, tempQueue);
        }

        // Notify waiting consumers
        cvConsumer.notify_one();
        
        return true;
    }

    bool consume(T& data, int timeout = 0) {
        std::unique_lock<std::mutex> lock(mutex);

        // Wait for data if the queue is empty
        if (dataQueue.empty()) {
            if (timeout >= 0) {
                if (cvConsumer.wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
                    // Timeout occurred
                    //std::cerr << "Consumer timed out." << std::endl;
                    return false;
                }
            } else {
                cvConsumer.wait(lock);
            }
        }

        // Retrieve the data from the queue
        if (!dataQueue.empty()) {
            data = dataQueue.front();
            dataQueue.pop();
            // Notify waiting producers
            cvProducer.notify_one();
            return true;
        }

        return false;
    }
};
