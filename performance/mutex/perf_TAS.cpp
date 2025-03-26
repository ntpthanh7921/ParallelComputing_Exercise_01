#include <chrono>
#include <implementation/mutex_competitive.hpp>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono;

bool is_prime(int p)
{
    for (int d = 2; d < p; d++)
        if (p % d == 0)
            return 0;
    return 1;
}

Spinlock_Mutex_TAS mtx;

void thread_entry(int num_iters, int inside_work, int outside_work, int check_target)
{
    for (auto i = 0; i < num_iters; i++)
    {
        mtx.lock();

        for (auto j = 0; j < inside_work; j++)
            is_prime(check_target);

        mtx.unlock();

        for (auto j = 0; j < outside_work; j++)
            is_prime(check_target);
    }
}

void run_test(int num_threads, int num_iters, int inside_work, int outside_work,
              int check_target = 991)
{
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(thread_entry, num_iters, inside_work, outside_work, check_target);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
}

int main(int argc, char *argv[])
{
    // Default parameter values
    int num_threads = 4;
    int num_iters = 1000;
    int inside_work = 100;
    int outside_work = 100;
    int check_target = 991;

    // Parse command-line arguments
    if (argc > 1)
        num_threads = std::stoi(argv[1]);
    if (argc > 2)
        num_iters = std::stoi(argv[2]);
    if (argc > 3)
        inside_work = std::stoi(argv[3]);
    if (argc > 4)
        outside_work = std::stoi(argv[4]);
    if (argc > 5)
        check_target = std::stoi(argv[5]);

    // Display run parameters
    std::cout << "Running test with parameters:" << std::endl;
    std::cout << "  Number of threads: " << num_threads << std::endl;
    std::cout << "  Iterations per thread: " << num_iters << std::endl;
    std::cout << "  Inside work: " << inside_work << std::endl;
    std::cout << "  Outside work: " << outside_work << std::endl;
    std::cout << "  Check target: " << check_target << std::endl;

    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    // Run the test
    run_test(num_threads, num_iters, inside_work, outside_work, check_target);

    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Report results
    std::cout << "Total execution time: " << duration.count() << " milliseconds" << std::endl;

    return 0;
}
