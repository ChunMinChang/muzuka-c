#ifndef DynamicRingBuffer_h
#define DynamicRingBuffer_h

#include "ring_buffer.h"
#include <vector>
#include <utility>
#include <memory>

template<typename T>
class DynamicRingBuffer {
public:
    explicit DynamicRingBuffer(size_t capacity)
        : batch_size_base(1)
        , threshold_base(capacity / 2)
        , buffer(capacity) {
        assert(capacity);
        // make sure capacity is power of two
        assert((capacity & (capacity - 1)) == 0);
        assert(threshold_base <= capacity && threshold_base > 0);
    }

    ~DynamicRingBuffer() = default;

    // Runs on producer thread
    void write(T&& data) {
        size_t writables = buffer.writable_capacity();

        // If this is the first write() after read_all()
        if (writables == buffer.capacity()) {
            submit_onhold_data();
            threshold = threshold_base;
            batch_size = batch_size_base;
        }

        if (!onhold) {
            onhold = std::make_unique<Batch>();
            onhold->set_capacity(batch_size);
        }

        // If the ring buffer is full, put the data into the onhold area,
        // no matter it is full or not
        if (writables == 0) {
            onhold->write(std::move(data));
            return;
        }

        // Put data into the onhold area
        onhold->write(std::move(data));
        if (onhold->is_full()) {
            submit_onhold_data();
            // Check if we need to throttle the producing rate
            // by enlarging the batch size
            if (writables - 1 <= threshold) {
                assert(!onhold); // Must happen after submitting a onhold
                threshold /= 2;
                batch_size *= 2;
            }
        }
    }

    // Runs on producer thread
    bool drain_writes() {
        if (!onhold) {
            return true;
        }
        size_t writables = buffer.writable_capacity();
        if (writables == 0) {
            return false;
        }
        submit_onhold_data();
        return true;
    }

    // Runs on consumer thread
    std::vector<T> read_all() {
        std::vector<T> flatten;
        std::vector<Batch*> batches = buffer.read_all();
        for (size_t i = 0 ; i < batches.size() ; ++i) {
            std::vector<T>& batch = batches[i]->data();
            flatten.insert(flatten.end(), batch.begin(), batch.end());
            delete batches[i];
        }
        return flatten;
    }

private:
    void submit_onhold_data() {
        if (onhold) {
            size_t written = buffer.write(onhold.release());
            assert(written > 0);
        }
        assert(!onhold);
    }

    class Batch {
    public:
        Batch() = default;
        ~Batch() = default;

        void set_capacity(size_t cap) {
            assert(cap);
            capacity = cap;
            buf.reserve(capacity);
        }

        bool is_full() {
            return buf.size() >= capacity;
        }

        void clear() {
            buf.clear();
        }

        void write(T&& data) {            
            buf.emplace_back(data);
        }

        std::vector<T>& data() {
            return buf;
        }

    private:
        size_t capacity;
        std::vector<T> buf;
    };

    size_t batch_size_base = 0;
    size_t batch_size = 0;
    std::unique_ptr<Batch> onhold;

    size_t threshold_base = 0;
    size_t threshold = 0;
    SPSCRingBuffer<Batch*> buffer;
};

#endif // DynamicRingBuffer_h