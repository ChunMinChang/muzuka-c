#ifndef RingBuffer_h
#define RingBuffer_h

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstring>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

// SPSCRingBuffer
//     A thread-safe single-producer-single-consumer circular buffer class.
//     The implementation has a read-cursor and a write-cursor to split up its
//     internal circular buffer into readable buffer and writable buffer. Both
//     read and write cursors are atomic variables so they can be accessed by
//     different threads safely. The atomic operations model here is based on
//     acquire-release ordering.
//
//     The range of the readable buffer is in [read-cursor, write-cursor) and
//     the range of the writable buffer is in [write-cursor, read-cursor - 1).
//     They are illustrated by the cells marked with '*' and the blank cells in
//     the figure below.
//
//     The read-cursor and write-cursor indicate the next available index to
//     read and write respectively. When read-cursor and write-cursor point to
//     the same position, the buffer is empty. When the write-cursor is one
//     step behind the read-cursor, the buffer is full (e.g., write-cursor
//     points to 'end' in the figure below). We cannot write the data to the
//     circular buffer when write-cursor is one step behind the read-cursor.
//     Otherwise, the write-cursor will point to the same position where 
//     read-cursor is after write operation and therefore we have no way to
//     tell if buffer is empty or full when write-cursor == read-cursor. Thus,
//     we always allocate one more byte for the end mark (read-cursor - 1) in
//     our internal circular buffer.
//
//           read-cursor
//               |
//          end  |
//           v   v
//     +---+---+---+---     ---+---+---+---+
//     |   | # | * |    ...    | * | * | * |
//     +---+---+---+---     ---+---+---+---+
//     |   |                           | * |
//     +---+                           +---+
//     |   |                           | * |
//     +---+                           +---+
//     |   |                           | * |
//       .                               .
//       .                               .
//     |   |                           | * |
//     +---+                           +---+
//     |   |                           | * |
//     +---+                           +---+
//     |   |                           |   | <- write-cursor
//     +---+---+---+---     ---+---+---+---+
//     |   |   |   |    ...    |   |   |   |
//     +---+---+---+---     ---+---+---+---+
template<class T>
class SPSCRingBuffer {
public:
    SPSCRingBuffer(size_t capacity): write_index(0), read_index(0) {
        assert(capacity > 0);
        // Make sure computation in advance_index(...) won't ovewrflow
        assert(capacity < std::numeric_limits<size_t>::max() / 2);
        buffer = std::vector<T>(capacity + 1);
        // Make sure constructor is always built first
        std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);
    };

    ~SPSCRingBuffer() = default;

    // Runs on producer thread
    size_t write(const T& data) {
        return write(&data, 1);
    }

    // Runs on producer thread
    size_t write_all(const std::vector<T>& data) {
        return write(data.data(), data.size());
    }

    // Runs on consumer thread
    std::optional<T> read() {
        std::vector<T> data = read(1);
        if (data.empty()) {
            return std::nullopt;
        }
        return std::optional<T>(std::move(data[0]));
    }

    // Runs on consumer thread
    std::vector<T> read_all() {
        return read(capacity());
    }
private:
    // Runs on producer thread
    size_t write(const T* data, size_t count) {
        // Transitive Synchronication with Acquire-Release Ordering:
        //     If read_index.store(...) has been called by read(...) on
        //     consumer thread, the read_index and the write_index will be
        //     newest value. Since write_index is only updated here, so wr_idx
        //     value is same as what read(...) reads.
        //
        //     If read_index.store(...) has not been called yet, it's fine.
        //     Our implementation guarantees the newer read_index will be always
        //     ahead of older read_index, and write_index is always behind the
        //     read_index. The smaller difference between read_index and
        //     write_index, the smaller data we can read.
        size_t rd_idx = read_index.load(std::memory_order::memory_order_acquire);
        size_t wr_idx = write_index.load(std::memory_order::memory_order_relaxed);

        if (is_full(rd_idx, wr_idx) || count == 0) {
            return 0;
        }

        size_t availables = writable(rd_idx, wr_idx);
        assert(availables > 0 && availables <= capacity());
        size_t num = std::min(count, availables);

        // Write the data
        assert(data);
        // first part: from the wr_idx to the end of the buffer
        size_t first_part = std::min(buffer.size() - wr_idx, num);
        copy(buffer.data() + wr_idx, data, first_part);
        // second part: from the beginning of the buffer
        size_t second_part = num - first_part;
        copy(buffer.data(), data + first_part, second_part);

        // Update write_index
        write_index.store(advance_index(wr_idx, num),
                          std::memory_order::memory_order_release);

        return num;
    }
    // Runs on consumer thread
    std::vector<T> read(size_t count) {
        // Transitive Synchronication with Acquire-Release Ordering:
        //     If write_index.store(...) has been called by write(...) on
        //     producer thread, the write_index and the read_index will be
        //     newest value. Since read_index is only updated here, so rd_idx
        //     value is same as what write(...) reads.
        //
        //     If write_index.store(...) has not been called yet, it's fine.
        //     Our implementation guarantees the newer write_index will
        //     always be ahead of older write_index, and write_index will always
        //     be in front of the read_index. The smaller difference between
        //     write_index and read_index, the smaller data we can read.
        size_t wr_idx = write_index.load(std::memory_order::memory_order_acquire);
        size_t rd_idx = read_index.load(std::memory_order::memory_order_relaxed);

        if (is_empty(rd_idx, wr_idx) || count == 0) {
            return {};
        }

        size_t availables = readable(rd_idx, wr_idx);
        assert(availables > 0 && availables <= capacity());
        size_t num = std::min(count, availables);

        // Read the data
        std::vector<T> values(num);
        // first part: from the rd_idx to the end of the buffer
        size_t first_part = std::min(buffer.size() - rd_idx, num);
        copy(values.data(), buffer.data() + rd_idx, first_part);
        // second part: from the beginning of the buffer
        size_t second_part = num - first_part;
        copy(values.data() + first_part, buffer.data(), second_part);

        // Update read_index
        read_index.store(advance_index(rd_idx, num),
                         std::memory_order::memory_order_release);

        return values;
    }

    size_t advance_index(size_t idx, size_t advancement) const {
        assert(idx < buffer.size());
        assert(advancement <= capacity());
        return (idx + advancement) % buffer.size();
    }

    size_t writable(size_t rd_idx, size_t wr_idx) const {
        return capacity() - readable(rd_idx, wr_idx);
    }

    size_t readable(size_t rd_idx, size_t wr_idx) const {
        assert(rd_idx < buffer.size());
        assert(wr_idx < buffer.size());
        return wr_idx >= rd_idx ? wr_idx - rd_idx
                                : capacity() - (rd_idx - wr_idx - 1);
    }

    bool is_empty(size_t rd_idx, size_t wr_idx) const {
        return wr_idx == rd_idx;
    }

    bool is_full(size_t rd_idx, size_t wr_idx) const {
        return (wr_idx + 1) % buffer.size() == rd_idx;
    }

    size_t capacity() const {
        assert(buffer.size() > 0);
        return buffer.size() - 1;
    }

    static inline void copy(T* dst, const T* src, size_t elem) {
        // Make sure destination and source isn't overlapped
        assert(dst + elem <= src || src + elem <= dst);
        std::memcpy(dst, src, elem * sizeof(T));
    }

    std::vector<T> buffer;
    std::atomic<std::size_t> write_index; // next available index to write
    std::atomic<std::size_t> read_index; // next available index to read
};

#endif // RingBuffer_h