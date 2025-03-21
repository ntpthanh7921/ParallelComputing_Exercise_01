#pragma once

#include <atomic>
#include <chrono>
#include <implementation/cpu_pause.hpp>
#include <thread>

using namespace std::chrono_literals;
constexpr std::chrono::microseconds MAX_DELAY = 10000us;
constexpr std::chrono::nanoseconds STARTING_DELAY = 100ns;

class Centralized_Barrier
{
public:
    // arrive at the barrier, wait to be let through, blocking
    virtual void wait() noexcept = 0;

    // need this for polymorphism of different barrier type
    // see: https://stackoverflow.com/questions/461203/when-to-use-virtual-destructors
    virtual ~Centralized_Barrier() { };
};

class Barrier_SR : public Centralized_Barrier
{
private:
    std::atomic<int> count;
    bool sense;
    const int limit;

public:

    Barrier_SR(int _limit) : count(_limit), limit(_limit) { }

    // Blocking wait
    void wait() noexcept
    {
        // Local sense variable, each thread have their own version of this
        bool local_sense = !sense;

        if (count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            count.store(limit, std::memory_order_release);
            sense = local_sense;
        }
        else
        {
            while (sense != local_sense)
            {
                // Spinning, waiting for enough threads to wait at barrier
                cpu_pause();
            }
        }
    }
};

class Barrier_SR_EBO : public Centralized_Barrier
{
private:
    std::atomic<int> count;
    bool sense;
    const int limit;

public:

    Barrier_SR_EBO(int _limit) : count(_limit), limit(_limit) { }

    // Blocking wait
    void wait() noexcept
    {
        // Per call to lock create different version of curr_delay on stack
        // so no fear of data race, because each thread have its own
        std::chrono::nanoseconds curr_delay = STARTING_DELAY;

        // Local sense variable, each thread have their own version of this
        bool local_sense = !sense;

        if (count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            count.store(limit, std::memory_order_release);
            sense = local_sense;
        }
        else
        {
            while (sense != local_sense)
            {
                // Spinning, waiting for enough threads to wait at barrier
                cpu_pause();
                std::this_thread::sleep_for(curr_delay);
                if (curr_delay < MAX_DELAY)
                    curr_delay *= 2;
            }
        }
    }
};
