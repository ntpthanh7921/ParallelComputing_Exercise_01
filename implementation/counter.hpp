#pragma once

#include <atomic>
#include <chrono>
#include <implementation/cpu_pause.hpp>
#include <thread>
#include <mutex>

using namespace std::chrono_literals;
constexpr std::chrono::microseconds MAX_DELAY = 10000us;
constexpr std::chrono::nanoseconds STARTING_DELAY = 100ns;

class Concurrent_Counter {
    public:
        // virtual void increment() noexcept = 0;
        // virtual int get_value() noexcept = 0;

        // need this for polymorphism of different barrier type
        // see: https://stackoverflow.com/questions/461203/when-to-use-virtual-destructors
        virtual ~Concurrent_Counter() { };
};

class Counter_MB : public Concurrent_Counter {
    // Mutex-Based Concurrent Counter
    private:
        int counter_mutex = 0;
        std::mutex mtx;

    public:
        Counter_MB(int _init) : counter_mutex(_init) { }

        void increment() {
            std::lock_guard<std::mutex> lock(mtx);
            counter_mutex++;
        }

        int get_value() {
            return counter_mutex;
        }

        void set_value(int _value) {
            counter_mutex = _value;
        }
};

class Counter_CAS : public Concurrent_Counter {
    // Compare-and-Swap Concurrent Counter
    private:
        std::atomic<int> counter_cas = 0;

    public:
        Counter_CAS(int _init) : counter_cas(_init) { }

        void increment() {
            int expected = counter_cas.load();
            while (!counter_cas.compare_exchange_weak(expected, expected + 1)) {
                expected = counter_cas.load();  // Reload expected value if CAS fails
            }
        }

        int get_value() {
            return counter_cas;
        }

        void set_value(int _value) {
            counter_cas = _value;
        }
};

class Counter_FAD : public Concurrent_Counter {
    // Fetch-and-Add Concurrent Counter
    private:
        std::atomic<int> counter_atomic = 0;

    public:
        Counter_FAD(int _init) : counter_atomic(_init) { }

        void increment() {
            counter_atomic.fetch_add(1, std::memory_order_relaxed);
        }

        int get_value() {
            return counter_atomic;
        }

        void set_value(int _value) {
            counter_atomic = _value;
        }
};