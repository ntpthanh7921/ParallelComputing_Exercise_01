#include <chrono>
#include <implementation/counter.hpp>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::chrono;

Counter_MB counter_mb = 0;

void thread_entry(int check_target) {
    for (auto j = 0; j < check_target; j++) 
        counter_mb.increment();
}

void run_test(int num_threads, int num_iters, int check_target) {
    for (auto i = 0; i < num_iters; i++) {
        std::vector<std::thread> threads;

        counter_mb.set_value(0);
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(thread_entry, check_target);
        }

        for (auto &thread : threads) {
            thread.join();
        }
    }
}

int main(int argc, char *argv[])
{
    // Default parameter values
    int num_threads = 4;
    int num_iters = 1000;
    int check_target = 10000;

    // Parse command-line arguments
    if (argc > 1)
        num_threads = std::stoi(argv[1]);
    if (argc > 2)
        num_iters = std::stoi(argv[2]);
    if (argc > 3)
        check_target = std::stoi(argv[5]);

    // Display run parameters
    std::cout << "Running test with parameters:" << std::endl;
    std::cout << "  Number of threads: " << num_threads << std::endl;
    std::cout << "  Iterations per thread: " << num_iters << std::endl;
    std::cout << "  Check target: " << check_target << std::endl;

    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    // Run the test
    run_test(num_threads, num_iters, check_target);

    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Report results
    std::cout << "Total execution time: " << duration.count() << " milliseconds" << std::endl;

    return 0;
}
