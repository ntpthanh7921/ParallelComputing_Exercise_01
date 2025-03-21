#include <implementation/counter.hpp>
#include <iostream>
#include <thread>
#include <vector>

constexpr int TEST_ITER = 500;
constexpr int NUM_THREAD = 4;  // should be set of max thread count on system,
                               // higher -> lock waiter preemption problem -> REALLY bad perf
constexpr int INCREMENT_AMOUNT_PER_THREAD = 10000;

Counter_CAS counter_cas     = 0;
int         counter_normal  = 0;

void increment_thread_normal_counter(int iterations) {
    for (int j = 0; j < iterations; ++j) {
        counter_normal++;
    }
}

void increment_thread_concurrent_counter(int iterations) {
    for (int j = 0; j < iterations; ++j) {
        counter_cas.increment();
    }
}

bool test_run_increments_for_normal_counter(int num_threads, int iterations_per_thread) {
    counter_normal = 0;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_thread_normal_counter, iterations_per_thread);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    return counter_normal == (num_threads * iterations_per_thread);
}


bool test_run_increments_for_concurrent_counter(int num_threads, int iterations_per_thread) {
    counter_cas.set_value(0);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_thread_concurrent_counter, iterations_per_thread);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    return counter_cas.get_value() == (num_threads * iterations_per_thread);
}

int main() {
    int success_count_with_normal_counter = 0;
    int success_count_with_concurrent_counter = 0;
    for (int i = 0; i < TEST_ITER; i++) {
        success_count_with_normal_counter       += test_run_increments_for_normal_counter(NUM_THREAD, INCREMENT_AMOUNT_PER_THREAD);
        success_count_with_concurrent_counter   += test_run_increments_for_concurrent_counter(NUM_THREAD, INCREMENT_AMOUNT_PER_THREAD);
    }

    std::cout << "Test result:" << '\n'
              << "With normal counter: " << success_count_with_normal_counter << '/' << TEST_ITER << '\n'
              << "With concurrent counter CAS: " << success_count_with_concurrent_counter << '/' << TEST_ITER << '\n';

    if (success_count_with_concurrent_counter == TEST_ITER)
        std::cout << "[PASSED]" << std::endl;
    else
        std::cout << "[FAILED]" << std::endl;

    return 0;
}
