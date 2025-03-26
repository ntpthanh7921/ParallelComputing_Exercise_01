#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <omp.h>

class BarrierPerformanceTest
{
private:
    int num_threads;
    int total_phases;
    std::atomic<int> completed_phases{0};

public:
    BarrierPerformanceTest(int threads, int phases) : num_threads(threads), total_phases(phases)
    {
        // Set the number of OpenMP threads
        omp_set_num_threads(num_threads);
    }

    double run_test(double &elapsed_seconds)
    {
        auto start_time = std::chrono::high_resolution_clock::now();

// Using OpenMP parallel section
#pragma omp parallel
        {
            // local variable
            int local_count = 0;

            while (local_count < total_phases)
            {
// Synchronize all threads at this point
#pragma omp barrier
                local_count++;
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

    std::cout << "Running OpenMP barrier performance test with:" << std::endl;
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
