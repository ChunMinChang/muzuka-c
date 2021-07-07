#include "dynamic_ring_buffer.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <string>
#include <thread>

const size_t NUM_OF_MESSAGES = 256;
std::vector<std::string> MESSAGES;

const size_t CAPACITY = 16; // 2^4
// The threshold will be 8, 4, 2, 1, 0. The batch size will change (4+1) times
DynamicRingBuffer<std::string> RING_BUFFER(CAPACITY);
// By the above setting, RING_BUFFER should never be full
// when the producing-rate/consuming-rate < (k/2 + 1) * 2^k = 3 * 16 = 48
const int PRODUCER_DELAY = 1;
const int CONSUMER_DELAY = 2 * CAPACITY;

std::atomic<bool> GO(false);

void producer() {
    // Ensure that the threads all start as near to the same time as possible
    while (!GO);

    for (size_t i = 0; i < NUM_OF_MESSAGES ; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(PRODUCER_DELAY));
        RING_BUFFER.write(std::to_string(i));
    }

    while(!RING_BUFFER.drain_writes());
}

void consumer() {
    // Ensure that the threads all start as near to the same time as possible
    while (!GO);

    while (MESSAGES.size() < NUM_OF_MESSAGES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(CONSUMER_DELAY));
        std::vector<std::string> data = RING_BUFFER.read_all();
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

    // Check if we drop any data
    assert(MESSAGES.size() == NUM_OF_MESSAGES);
    for (size_t i = 0 ; i < MESSAGES.size() ; ++i) {
        assert(MESSAGES[i].compare(std::string(std::to_string(i))) == 0);
    }

    return 0;
}