#include <chrono>
#include <implementation/counter.hpp>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::chrono;

Counter_FAD counter_fad = 0;

void thread_entry(int counter_target_per_thread) {
    for (auto j = 0; j < counter_target_per_thread; j++) 
        counter_fad.increment();
}

void run_test(int num_threads, int num_iters, int counter_target_per_thread) {
    for (auto i = 0; i < num_iters; i++) {
        std::vector<std::thread> threads;

        counter_fad.set_value(0);
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(thread_entry, counter_target_per_thread);
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
    int counter_target_per_thread = 20000;

    // Parse command-line arguments
    if (argc > 1)
        num_threads = std::stoi(argv[1]);
    if (argc > 2)
        num_iters = std::stoi(argv[2]);
    if (argc > 3)
        counter_target_per_thread = std::stoi(argv[3]);

    // Display run parameters
    std::cout << "Running test with parameters:" << std::endl;
    std::cout << "  Number of threads: " << num_threads << std::endl;
    std::cout << "  Iterations per thread: " << num_iters << std::endl;
    std::cout << "  Counter target per thread: " << counter_target_per_thread << std::endl;

    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    // Run the test
    run_test(num_threads, num_iters, counter_target_per_thread);

    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Report results
    std::cout << "Total execution time: " << duration.count() << " milliseconds" << std::endl;

    return 0;
}
