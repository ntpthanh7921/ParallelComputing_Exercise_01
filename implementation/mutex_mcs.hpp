#pragma once

#include "cpu_pause.hpp"
#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
constexpr std::chrono::microseconds MAX_DELAY = 10000us;
constexpr std::chrono::microseconds STARTING_DELAY = 1us;

// implements a linked list waiting queue, according to
// Algorithms for scalable synchronization on shared-memory multiprocessors (1991)
// (see paper Reference)
class Spinlock_Mutex_MCS
{
public:
    struct QueueNode
    {
        std::atomic<QueueNode *> next = nullptr;
        std::atomic<bool> is_locked = true;
    };

private:
    std::atomic<QueueNode *> tail = nullptr;

public:

    // blocking
    void lock(QueueNode *pending) noexcept
    {
        std::chrono::microseconds curr_delay = STARTING_DELAY;

        pending->next.store(nullptr, std::memory_order_release);
        QueueNode *predecessor = tail.exchange(pending, std::memory_order_acq_rel);
        if (predecessor != nullptr)
        {
            pending->is_locked.store(true, std::memory_order_release);
            predecessor->next.store(pending, std::memory_order_release);
            while (pending->is_locked.load(std::memory_order_acquire))
            {
                // Spinning, so use cpu_pause()
                cpu_pause();
                // std::this_thread::sleep_for(curr_delay);
                // if (curr_delay < MAX_DELAY)
                //     curr_delay *= 2;
            }
        }
    }

    void unlock(QueueNode *pending) noexcept
    {
        std::chrono::microseconds curr_delay = STARTING_DELAY;
        auto temp = pending;  // use in cas to prevent pending being changed

        if (pending->next.load(std::memory_order_acquire) == nullptr)
        {
            if (tail.compare_exchange_strong(temp, nullptr, std::memory_order_acq_rel,
                                             std::memory_order_relaxed))
                return;
            while (pending->next.load(std::memory_order_acquire) == nullptr)
            {
                // Spinning, so use cpu_pause()
                cpu_pause();
                // std::this_thread::sleep_for(curr_delay);
                // if (curr_delay < MAX_DELAY)
                //     curr_delay *= 2;
            }
        }
        pending->next.load(std::memory_order_acquire)
            ->is_locked.store(false, std::memory_order_release);
    }

    // To use the lock, use will have to allocate a QueueNode
    // This is like a ticket, which will be used to try to lock/unlock
    QueueNode *allocate_QueueNode() { return new QueueNode(); }

    // User will have to deallocate the QueueNode to avoid memory leak
    void deallocate_QueueNode(QueueNode *pending) { delete pending; }
};
