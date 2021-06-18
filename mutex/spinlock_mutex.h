#ifndef SpinlockMutex_h
#define SpinlockMutex_h

#include <atomic>

class SpinlockMutex {
public:
    SpinlockMutex(): flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

#endif // SpinlockMutex_h