#include <atomic>
#include <barrier>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

class BarrierPerformanceTest
{
private:
    int num_threads;
    int total_phases;
    std::atomic<int> completed_phases{0};

public:
    BarrierPerformanceTest(int threads, int phases) : num_threads(threads), total_phases(phases) { }

    double run_test(double &elapsed_seconds)
    {
        auto start_time = std::chrono::high_resolution_clock::now();

        std::barrier sync_point(num_threads, [&]() noexcept { });

        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(
                [&]()
                {
                    int local_count = 0;

                    // Each thread will synchronize until all phases completed
                    while (local_count < total_phases)
                    {
                        sync_point.arrive_and_wait();
                        local_count++;
                    }
                });
        }

        // Join all threads after completion
        for (auto &t : threads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;

        elapsed_seconds = elapsed.count();
        return static_cast<double>(total_phases) / elapsed_seconds;  // phases per second
    }
};

int main(int argc, char *argv[])
{
    // Default parameters
    int num_threads = 4;
    int total_phases = 100'000;

    // Parse command line arguments if provided
    if (argc > 1)
    {
        num_threads = std::stoi(argv[1]);
    }
    if (argc > 2)
    {
        total_phases = std::stoi(argv[2]);
    }

    std::cout << "Running barrier performance test with:" << std::endl;
    std::cout << "  - " << num_threads << " threads" << std::endl;
    std::cout << "  - " << total_phases << " total phases" << std::endl;

    double elapsed_seconds = 0.0;
    BarrierPerformanceTest test(num_threads, total_phases);
    double phases_per_second = test.run_test(elapsed_seconds);

    std::cout << "Results:" << std::endl;
    std::cout << "  - Completed " << total_phases << " phases" << std::endl;
    std::cout << "  - Total time: " << std::fixed << std::setprecision(4) << elapsed_seconds
              << " seconds" << std::endl;
    std::cout << "  - Performance: " << std::fixed << std::setprecision(2) << phases_per_second
              << " phases per second" << std::endl;

    return 0;
}
