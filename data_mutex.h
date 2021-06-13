#ifndef DataMutex_h
#define DataMutex_h

#include <cassert>
#include <cstdio>
#include <mutex>

// This is a Rust-style mutex [1] written in C++
// Usage:
//
//    DataMutex<uint32_t> shared(100);
//    {
//        auto guard = shared.lock(); // Enter critical section
//        guard.data() += 1;
//    } // Leave critical section
//
//    {
//        auto guard = shared.lock(); // Enter critical section
//        assert(guard.data(), 101);
//    } // Leave critical section
//
// [1] https://doc.rust-lang.org/std/sync/struct.Mutex.html
template<class T>
class DataMutex final {
public:
    // Prefer allocating the shared resource inside this class directly, by 
    // move constructor, to prevent accessing the resource without lock.
    DataMutex(T&& d): data(d) {}
    ~DataMutex() = default;

    // RAII style lock returned from DataMutex::lock().
    class MutexGuard final {
    public:
        MutexGuard(MutexGuard&& other) : owner(other.owner) {
          other.owner = nullptr;
        }

        ~MutexGuard() {
            if (owner) {
                owner->mutex.unlock();
            }
        }

        T& data() {
            return owner->data;
        }
    private:
        friend class DataMutex;
        MutexGuard(const MutexGuard& other) = delete;

        explicit MutexGuard(DataMutex<T>* o):owner(o) {
            assert(owner);
            owner->mutex.lock();
        }

        DataMutex<T>* owner;
    };

    MutexGuard lock() {
        return MutexGuard(this);
    }

private:
    std::mutex mutex;
    T data;
};

#endif // DataMutex_h