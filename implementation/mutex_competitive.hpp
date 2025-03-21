#pragma once

#include "cpu_pause.hpp"
#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
constexpr std::chrono::microseconds MAX_DELAY = 10000us;
constexpr std::chrono::nanoseconds STARTING_DELAY = 100ns;

class Spinlock_Mutex
{
public:
    // try to acquire lock, blocking
    virtual void lock() noexcept = 0;

    // release the lock
    virtual void unlock() noexcept = 0;

    // try to acquire lock, non-blocking, return false if fail
    // return true if success in acquiring
    virtual bool try_lock() noexcept = 0;

    // need this for polymorphism of different lock type
    // see: https://stackoverflow.com/questions/461203/when-to-use-virtual-destructors
    virtual ~Spinlock_Mutex() { };
};

class Spinlock_Mutex_TAS : public Spinlock_Mutex
{
private:
    std::atomic_flag flag_;

public:
    void lock() noexcept
    {
        while (flag_.test_and_set(std::memory_order_acquire))
        {
            // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
            // hyper-threads
            cpu_pause();
        }
    }

    void unlock() noexcept { flag_.clear(std::memory_order_release); }

    bool try_lock() noexcept
    {
        // Non-blocking, but still try to test_and_set flag_,
        // so have bad cache behavior in high contention case, try not to use this
        return !flag_.test_and_set(std::memory_order_acquire);
    }
};

// implementation taken from https://rigtorp.se/spinlock/
//
class Spinlock_Mutex_TTAS : public Spinlock_Mutex
{
private:
    std::atomic<bool> lock_;

public:
    void lock() noexcept
    {
        while (true)
        {
            // Optimistically assume the lock is free on the first try
            if (!lock_.exchange(true, std::memory_order_acquire))
            {
                return;
            }

            // If lock is not free, spin with load
            while (lock_.load(std::memory_order_relaxed))
            {
                // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                // hyper-threads
                cpu_pause();
            }
        }
    }

    void unlock() noexcept { lock_.store(false, std::memory_order_release); }

    bool try_lock() noexcept
    {
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        return !lock_.load(std::memory_order_relaxed)
               && !lock_.exchange(true, std::memory_order_acquire);
    }
};

class Spinlock_Mutex_TAS_EBO : public Spinlock_Mutex
{
private:
    std::atomic_flag flag_;

public:
    void lock() noexcept
    {
        // Per call to lock create different version of curr_delay on stack
        // so no fear of data race, because each thread have its own
        std::chrono::nanoseconds curr_delay = STARTING_DELAY;

        while (flag_.test_and_set(std::memory_order_acquire))
        {
            // Sleep the thread for curr_delay before trying to lock again
            // This avoid cache invalidation in cache coherence protocol
            // It also reduce memory bandwidth of thread when spinning
            cpu_pause();
            std::this_thread::sleep_for(curr_delay);
            if (curr_delay < MAX_DELAY)
                curr_delay *= 2;
        }
    }

    void unlock() noexcept { flag_.clear(std::memory_order_release); }

    bool try_lock() noexcept
    {
        // this will not implement exponential backoff so use this carefullly
        return !flag_.test_and_set(std::memory_order_acquire);
    }
};

class Spinlock_Mutex_TTAS_EBO : public Spinlock_Mutex
{
private:
    std::atomic<bool> lock_;

public:
    void lock() noexcept
    {
        // Per call to lock create different version of curr_delay on stack
        // so no fear of data race, because each thread have its own
        std::chrono::nanoseconds curr_delay = STARTING_DELAY;

        while (true)
        {
            // Optimistically assume the lock is free on the first try
            if (!lock_.exchange(true, std::memory_order_acquire))
            {
                return;
            }

            // If lock is not free, spin with load
            while (lock_.load(std::memory_order_relaxed))
            {
                // Sleep the thread for curr_delay before trying check lock again
                // It also reduce memory bandwidth of thread when spinning
                cpu_pause();
                std::this_thread::sleep_for(curr_delay);
                if (curr_delay < MAX_DELAY)
                    curr_delay *= 2;
            }
        }
    }

    void unlock() noexcept { lock_.store(false, std::memory_order_release); }

    bool try_lock() noexcept
    {
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        return !lock_.load(std::memory_order_relaxed)
               && !lock_.exchange(true, std::memory_order_acquire);
    }
};
