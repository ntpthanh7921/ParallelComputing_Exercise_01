#include <implementation/mutex_competitive.hpp>
#include <iostream>
#include <thread>
#include <vector>

constexpr int TEST_ITER = 500;
constexpr int NUM_THREAD = 4;
constexpr int INCREMENT_AMOUNT_PER_THREAD = 10000;

int counter = 0;
Spinlock_Mutex_TTAS mtx;

void increment_thread_without_lock(int iterations)
{
    for (int j = 0; j < iterations; ++j)
    {
        counter++;
    }
}

void increment_thread_with_lock(int iterations)
{
    for (int j = 0; j < iterations; ++j)
    {
        // Manual mutex locking and unlocking
        mtx.lock();
        counter++;
        mtx.unlock();
    }
}

bool test_run_increments(int num_threads, int iterations_per_thread, void increment_function(int))
{
    counter = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(increment_function, iterations_per_thread);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    return counter == (num_threads * iterations_per_thread);
}

int main()
{

    int success_count_without_lock = 0;
    int success_count_with_lock = 0;
    for (int i = 0; i < TEST_ITER; i++)
    {
        success_count_without_lock += test_run_increments(NUM_THREAD, INCREMENT_AMOUNT_PER_THREAD,
                                                          increment_thread_without_lock);
        success_count_with_lock += test_run_increments(NUM_THREAD, INCREMENT_AMOUNT_PER_THREAD,
                                                       increment_thread_with_lock);
    }

    std::cout << "Test result:" << '\n'
              << "Without lock: " << success_count_without_lock << '/' << TEST_ITER << '\n'
              << "With lock: " << success_count_with_lock << '/' << TEST_ITER << '\n';

    if (success_count_with_lock == TEST_ITER)
        std::cout << "[PASSED]" << std::endl;
    else
        std::cout << "[FAILED]" << std::endl;

    return 0;
}
