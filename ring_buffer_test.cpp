#include "ring_buffer.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

const size_t NUM_OF_MESSAGES = 100;
SPSCRingBuffer<std::string> SPSC_QUEUE(NUM_OF_MESSAGES/10);
std::vector<std::string> MESSAGES;

std::atomic<bool> GO(false);

void producer() {
    const size_t batch_size = 7;

    // Ensure that the threads all start as near to the same time as possible
    while (!GO);

    size_t id = 0;
    std::vector<std::string> data;
    while (id < NUM_OF_MESSAGES) {
        if (data.empty()) {
            for (size_t i = 0 ; i < batch_size ; ++i) {
                data.emplace_back(std::to_string(id + i));
            }
        }
        bool use_write_all = id < (NUM_OF_MESSAGES * 3 / 5);
        size_t written = use_write_all ? SPSC_QUEUE.write_all(data)
                                       : SPSC_QUEUE.write(data.front());

        std::cout << "produce: ";
        if (written == 0) {
            std::cout << "nothing (full)" << std::endl;
            continue;
        }
        std::reverse(data.begin(), data.end());
        for (size_t i = 0 ; i < written ; ++i) {
            std::cout << data.back() << " ";
            data.pop_back();
        }
        std::cout << std::endl;
        std::reverse(data.begin(), data.end());
        id += written;
    }
}

void consumer() {
    // Ensure that the threads all start as near to the same time as possible
    while (!GO);

    // Add some delays
    std::this_thread::sleep_for (std::chrono::milliseconds(1));

    while (MESSAGES.size() < NUM_OF_MESSAGES) {
        std::vector<std::string> data;
        bool use_read_all = MESSAGES.size() < (NUM_OF_MESSAGES * 4 / 5);
        if (use_read_all) {
            data = SPSC_QUEUE.read_all();
        } else {
            std::optional<std::string> message = SPSC_QUEUE.read();
            if (message.has_value()) {
                data.emplace_back(std::move(*message));
            }
        }

        std::cout << "consume: ";
        if (data.empty()) {
            std::cout << "nothing" << std::endl;
            continue;
        }
        for (const std::string& s: data) {
            std::cout << s << " ";
        }
        std::cout << std::endl;

        MESSAGES.insert(MESSAGES.end(), data.begin(), data.end());
    }
}

int main() {
    GO = false;

    std::thread t1(consumer);
    std::thread t2(producer);

    GO = true;

    t1.join();
    t2.join();

    assert(MESSAGES.size() == NUM_OF_MESSAGES);
    for (size_t i = 0 ; i < MESSAGES.size() ; ++i) {
        assert(MESSAGES[i].compare(std::string(std::to_string(i))) == 0);
    }

    return 0;
}