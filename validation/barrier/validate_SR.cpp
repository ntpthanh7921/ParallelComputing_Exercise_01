#include <atomic>
#include <implementation/barrier.hpp>
#include <iostream>
#include <thread>
#include <vector>

constexpr int TEST_ITER = 500;
constexpr int NUM_THREAD = 4;
constexpr int LOOP_WITHIN_EACH_TEST = 500;
constexpr int DIFF_LIMIT = 2;

Barrier_SR barrier(NUM_THREAD);
std::vector<int> global_counts(NUM_THREAD);
std::vector<std::atomic<bool>>
    global_results(NUM_THREAD);  // doesn't work if not atomic<bool>, not sure why

void increment_and_check_everyone_else(int idx, int iter)
{
    bool thread_result = true;

    // loop through 2*iter since inner loop only contains 1 phase
    // version with barrier will have 2 phase
    for (int i = 0; i < 2 * iter; i++)
    {
        // increment own counter
        global_counts[idx]++;

        // check everyone else counter
        for (int c : global_counts)
        {
            int diff = std::abs(global_counts[idx] - c);
            if (diff >= DIFF_LIMIT)
                thread_result = false;
        }
    }

    global_results[idx] = thread_result;
}

void increment_and_check_everyone_else_with_barrier(int idx, int iter)
{
    bool thread_result = true;
    for (int i = 0; i < iter; i++)
    {

        // increment own counter
        global_counts[idx]++;

        barrier.wait();

        // check everyone else counter
        for (int c : global_counts)
        {
            int diff = std::abs(global_counts[idx] - c);
            if (diff >= DIFF_LIMIT)
                thread_result = false;
        }

        // increment own counter
        global_counts[idx]++;

        barrier.wait();

        // check everyone else counter
        for (int c : global_counts)
        {
            int diff = std::abs(global_counts[idx] - c);
            if (diff >= DIFF_LIMIT)
                thread_result = false;
        }
    }

    global_results[idx] = thread_result;
}

bool run_test(int num_threads, int iterations_per_thread, void thread_func(int, int))
{
    std::vector<std::thread> threads;
    std::fill(global_counts.begin(), global_counts.end(), 0);
    std::fill(global_results.begin(), global_results.end(), false);

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(thread_func, i, iterations_per_thread);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    // AND over all result
    bool combined_result = true;  // Start with true (identity element for AND)
    for (bool value : global_results)
    {
        combined_result = combined_result && value;
        // Short-circuit: if we find a false, we can stop
        if (!value)
            break;
    }

    return combined_result;
}

int main()
{
    int success_count_without_barrier = 0;
    int success_count_with_barrier = 0;

    for (int i = 0; i < TEST_ITER; i++)
    {
        success_count_without_barrier +=
            run_test(NUM_THREAD, LOOP_WITHIN_EACH_TEST, &increment_and_check_everyone_else);
        success_count_with_barrier += run_test(NUM_THREAD, LOOP_WITHIN_EACH_TEST,
                                               &increment_and_check_everyone_else_with_barrier);
    }

    std::cout << "Test result:" << '\n'
              << "Without barrier: " << success_count_without_barrier << '/' << TEST_ITER << '\n'
              << "With barrier: " << success_count_with_barrier << '/' << TEST_ITER << '\n';

    if (success_count_with_barrier == TEST_ITER)
        std::cout << "[PASSED]" << std::endl;
    else
        std::cout << "[FAILED]" << std::endl;

    return 0;
}
