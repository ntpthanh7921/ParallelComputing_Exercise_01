#!/usr/bin/env python3

import subprocess
import pandas as pd
import re
import os
import argparse
from datetime import datetime
import numpy as np


def run_test(
    program_path, threads, iterations, inside_work, outside_work, check_target=991
):
    """Run the C++ program with specified parameters and parse the output."""
    cmd = [
        program_path,
        str(threads),
        str(iterations),
        str(inside_work),
        str(outside_work),
        str(check_target),
    ]

    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        output = result.stdout

        # Parse the execution time
        time_match = re.search(r"Total execution time: (\d+) milliseconds", output)
        if time_match:
            execution_time = int(time_match.group(1))
            return {
                "threads": threads,
                "iterations": iterations,
                "inside_work": inside_work,
                "outside_work": outside_work,
                "check_target": check_target,
                "execution_time_ms": execution_time,
                "output": output,
                "success": True,
            }
        else:
            return {
                "threads": threads,
                "iterations": iterations,
                "inside_work": inside_work,
                "outside_work": outside_work,
                "check_target": check_target,
                "execution_time_ms": -1,
                "output": output,
                "success": False,
                "error": "Could not parse execution time",
            }

    except subprocess.CalledProcessError as e:
        return {
            "threads": threads,
            "iterations": iterations,
            "inside_work": inside_work,
            "outside_work": outside_work,
            "check_target": check_target,
            "execution_time_ms": -1,
            "output": e.stdout,
            "error": e.stderr,
            "success": False,
        }


def generate_work_distributions(total_work, inside_work_percentages):
    """
    Generate inside_work and outside_work values based on total_work and percentages.

    Args:
        total_work: The sum of inside_work and outside_work
        inside_work_percentages: List of percentages for inside_work

    Returns:
        List of tuples (inside_work, outside_work)
    """
    distributions = []

    for percentage in inside_work_percentages:
        inside_work = int(round(total_work * percentage / 100))
        outside_work = total_work - inside_work
        # Ensure we don't have zero values
        inside_work = max(1, inside_work)
        outside_work = max(1, outside_work)
        # Adjust to match total_work exactly
        if inside_work + outside_work != total_work:
            outside_work = total_work - inside_work
        distributions.append((inside_work, outside_work))

    return distributions


def main():
    parser = argparse.ArgumentParser(
        description="Run performance tests for C++ program"
    )
    parser.add_argument("program_path", help="Path to the compiled C++ executable")
    parser.add_argument(
        "--threads",
        type=int,
        nargs="+",
        default=[1, 2, 4, 8, 16],
        help="List of thread counts to test",
    )
    parser.add_argument(
        "--iterations",
        type=int,
        nargs="+",
        default=[1000],
        help="List of iteration counts to test",
    )
    parser.add_argument(
        "--total-work",
        type=int,
        default=200,
        help="Total work (sum of inside_work and outside_work)",
    )
    parser.add_argument(
        "--inside-work-percentages",
        type=int,
        nargs="+",
        default=[10, 30, 50, 70, 90],
        help="Percentages of inside_work relative to total work",
    )
    parser.add_argument(
        "--check-target", type=int, default=991, help="Check target value"
    )
    parser.add_argument(
        "--test-name",
        type=str,
        default="",
        help="Name for this test run (used in output filenames)",
    )
    parser.add_argument(
        "--output-dir", type=str, default="./results", help="Directory for output files"
    )
    args = parser.parse_args()

    # Create output directory if it doesn't exist
    os.makedirs(args.output_dir, exist_ok=True)

    # Generate timestamp for filenames
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    # Create base filename using test name if provided
    if args.test_name:
        base_filename = f"{args.test_name}_{timestamp}"
    else:
        base_filename = f"performance_test_{timestamp}"

    # Set output filenames
    csv_filename = os.path.join(args.output_dir, f"{base_filename}.csv")
    report_filename = os.path.join(args.output_dir, f"{base_filename}_report.txt")
    log_filename = os.path.join(args.output_dir, f"{base_filename}_log.txt")

    # Generate work distributions
    work_distributions = generate_work_distributions(
        args.total_work, args.inside_work_percentages
    )

    # Generate all combinations of test parameters
    test_configs = []
    for threads in args.threads:
        for iterations in args.iterations:
            for inside_work, outside_work in work_distributions:
                test_configs.append((threads, iterations, inside_work, outside_work))

    total_tests = len(test_configs)
    print(
        f"Starting performance testing with {total_tests} different parameter combinations..."
    )
    print(f"Test name: {args.test_name if args.test_name else 'Not specified'}")
    print(f"Total work: {args.total_work}")
    print(f"Inside work percentages: {args.inside_work_percentages}")

    # Open log file
    with open(log_filename, "w") as log_file:
        log_file.write(
            f"Performance Test Log: {args.test_name if args.test_name else 'Unnamed test'}\n"
        )
        log_file.write(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        log_file.write(f"Program: {args.program_path}\n\n")
        log_file.write("Test Parameters:\n")
        log_file.write(f"  Threads: {args.threads}\n")
        log_file.write(f"  Iterations: {args.iterations}\n")
        log_file.write(f"  Total Work: {args.total_work}\n")
        log_file.write(f"  Inside Work Percentages: {args.inside_work_percentages}\n")
        log_file.write("  Generated Work Distributions:\n")
        for inside_work, outside_work in work_distributions:
            actual_percentage = (inside_work / (inside_work + outside_work)) * 100
            log_file.write(
                f"    Inside: {inside_work}, Outside: {outside_work} "
                f"(Inside percentage: {actual_percentage:.1f}%)\n"
            )
        log_file.write(f"  Check Target: {args.check_target}\n\n")
        log_file.write(f"Running {total_tests} tests...\n\n")

        results = []
        for i, (threads, iterations, inside_work, outside_work) in enumerate(
            test_configs, 1
        ):
            inside_percentage = (inside_work / (inside_work + outside_work)) * 100
            test_info = (
                f"Running test {i}/{total_tests}: threads={threads}, iterations={iterations}, "
                f"inside_work={inside_work} ({inside_percentage:.1f}%), "
                f"outside_work={outside_work} ({100 - inside_percentage:.1f}%)"
            )
            print(test_info)
            log_file.write(f"{test_info}\n")

            result = run_test(
                args.program_path,
                threads,
                iterations,
                inside_work,
                outside_work,
                args.check_target,
            )

            results.append(result)

            # Print progress info
            if result["success"]:
                progress_info = f"  Execution time: {result['execution_time_ms']} ms"
                print(progress_info)
                log_file.write(f"{progress_info}\n")
            else:
                error_info = f"  Test failed: {result.get('error', 'Unknown error')}"
                print(error_info)
                log_file.write(f"{error_info}\n")

            # Write full program output to log
            log_file.write("  Program output:\n")
            for line in result["output"].split("\n"):
                log_file.write(f"    {line}\n")
            log_file.write("\n")

    # Convert results to DataFrame and save to CSV
    df = pd.DataFrame(results)

    # Basic analysis
    successful_results = df[df["success"] == True].copy()
    if not successful_results.empty:
        # Calculate inside work percentage for each configuration
        successful_results["inside_percentage"] = (
            successful_results["inside_work"]
            / (successful_results["inside_work"] + successful_results["outside_work"])
            * 100
        )

        # Save to CSV
        cols_to_save = [
            "threads",
            "iterations",
            "inside_work",
            "outside_work",
            "inside_percentage",
            "check_target",
            "execution_time_ms",
            "success",
        ]
        successful_results[cols_to_save].to_csv(csv_filename, index=False)

        # Print some basic analysis
        print("\nPerformance Analysis:")
        print(f"Total tests run: {len(results)}")
        print(f"Successful tests: {len(successful_results)}")

        # Thread scaling analysis
        if len(args.threads) > 1:
            print("\nThread Scaling Analysis:")
            for inside, outside in work_distributions:
                subset = successful_results[
                    (successful_results["inside_work"] == inside)
                    & (successful_results["outside_work"] == outside)
                    & (successful_results["iterations"] == args.iterations[0])
                ]
                if not subset.empty:
                    inside_pct = (inside / (inside + outside)) * 100
                    print(
                        f"\nFor inside_work={inside} ({inside_pct:.1f}%), outside_work={outside} ({100 - inside_pct:.1f}%):"
                    )
                    thread_perf = subset[["threads", "execution_time_ms"]].sort_values(
                        "threads"
                    )
                    print(thread_perf)

                    # Calculate speedup relative to single thread
                    if 1 in args.threads:
                        single_thread_time = thread_perf[thread_perf["threads"] == 1][
                            "execution_time_ms"
                        ].values[0]
                        thread_perf["speedup"] = (
                            single_thread_time / thread_perf["execution_time_ms"]
                        )
                        print("\nSpeedup relative to single thread:")
                        print(thread_perf[["threads", "speedup"]])

        # Inside/outside work distribution analysis
        if len(args.inside_work_percentages) > 1:
            print("\nInside/Outside Work Distribution Analysis:")
            for thread in args.threads:
                for iter_count in args.iterations:
                    subset = successful_results[
                        (successful_results["threads"] == thread)
                        & (successful_results["iterations"] == iter_count)
                    ]
                    if not subset.empty:
                        print(f"\nFor threads={thread}, iterations={iter_count}:")
                        work_perf = subset[
                            [
                                "inside_work",
                                "outside_work",
                                "inside_percentage",
                                "execution_time_ms",
                            ]
                        ].sort_values("inside_percentage")
                        print(work_perf)

        # Generate a detailed report
        with open(report_filename, "w") as f:
            f.write(
                f"Performance Test Report: {args.test_name if args.test_name else 'Unnamed test'}\n"
            )
            f.write("=====================================================\n\n")
            f.write(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Program: {args.program_path}\n\n")

            f.write("Test Parameters:\n")
            f.write(f"  Threads: {args.threads}\n")
            f.write(f"  Iterations: {args.iterations}\n")
            f.write(f"  Total Work: {args.total_work}\n")
            f.write(f"  Inside Work Percentages: {args.inside_work_percentages}\n")
            f.write("  Generated Work Distributions:\n")
            for inside_work, outside_work in work_distributions:
                actual_percentage = (inside_work / (inside_work + outside_work)) * 100
                f.write(
                    f"    Inside: {inside_work}, Outside: {outside_work} "
                    f"(Inside percentage: {actual_percentage:.1f}%)\n"
                )
            f.write(f"  Check Target: {args.check_target}\n\n")

            f.write(f"Total tests run: {len(results)}\n")
            f.write(f"Successful tests: {len(successful_results)}\n\n")

            f.write("Results Summary:\n")
            f.write("---------------\n")
            f.write("Best performing configuration:\n")
            best = successful_results.loc[
                successful_results["execution_time_ms"].idxmin()
            ]
            f.write(f"  Threads: {best['threads']}\n")
            f.write(f"  Iterations: {best['iterations']}\n")
            f.write(
                f"  Inside Work: {best['inside_work']} ({best['inside_percentage']:.1f}%)\n"
            )
            f.write(
                f"  Outside Work: {best['outside_work']} ({100 - best['inside_percentage']:.1f}%)\n"
            )
            f.write(f"  Execution Time: {best['execution_time_ms']} ms\n\n")

            f.write("Worst performing configuration:\n")
            worst = successful_results.loc[
                successful_results["execution_time_ms"].idxmax()
            ]
            f.write(f"  Threads: {worst['threads']}\n")
            f.write(f"  Iterations: {worst['iterations']}\n")
            f.write(
                f"  Inside Work: {worst['inside_work']} ({worst['inside_percentage']:.1f}%)\n"
            )
            f.write(
                f"  Outside Work: {worst['outside_work']} ({100 - worst['inside_percentage']:.1f}%)\n"
            )
            f.write(f"  Execution Time: {worst['execution_time_ms']} ms\n\n")

            # Thread scaling analysis
            if len(args.threads) > 1:
                f.write("Thread Scaling Analysis:\n")
                f.write("------------------------\n")
                for inside, outside in work_distributions:
                    subset = successful_results[
                        (successful_results["inside_work"] == inside)
                        & (successful_results["outside_work"] == outside)
                        & (successful_results["iterations"] == args.iterations[0])
                    ]
                    if not subset.empty:
                        inside_pct = (inside / (inside + outside)) * 100
                        f.write(
                            f"\nFor inside_work={inside} ({inside_pct:.1f}%), outside_work={outside} ({100 - inside_pct:.1f}%):\n"
                        )
                        thread_perf = subset[
                            ["threads", "execution_time_ms"]
                        ].sort_values("threads")
                        for _, row in thread_perf.iterrows():
                            f.write(
                                f"  Threads: {row['threads']}, Time: {row['execution_time_ms']} ms\n"
                            )

                        # Calculate speedup relative to single thread
                        if 1 in args.threads:
                            f.write("\n  Speedup relative to single thread:\n")
                            single_thread_time = thread_perf[
                                thread_perf["threads"] == 1
                            ]["execution_time_ms"].values[0]
                            for _, row in thread_perf.iterrows():
                                speedup = single_thread_time / row["execution_time_ms"]
                                f.write(
                                    f"    Threads: {row['threads']}, Speedup: {speedup:.2f}x\n"
                                )
                        f.write("\n")

            # Inside/outside work distribution analysis
            if len(args.inside_work_percentages) > 1:
                f.write("\nInside/Outside Work Distribution Analysis:\n")
                f.write("----------------------------------------\n")
                for thread in args.threads:
                    for iter_count in args.iterations:
                        subset = successful_results[
                            (successful_results["threads"] == thread)
                            & (successful_results["iterations"] == iter_count)
                        ]
                        if not subset.empty:
                            f.write(
                                f"\nFor threads={thread}, iterations={iter_count}:\n"
                            )
                            work_perf = subset[
                                [
                                    "inside_work",
                                    "outside_work",
                                    "inside_percentage",
                                    "execution_time_ms",
                                ]
                            ].sort_values("inside_percentage")
                            for _, row in work_perf.iterrows():
                                f.write(
                                    f"  Inside: {row['inside_work']} ({row['inside_percentage']:.1f}%), "
                                    f"Outside: {row['outside_work']} ({100 - row['inside_percentage']:.1f}%), "
                                    f"Time: {row['execution_time_ms']} ms\n"
                                )

                            # Find optimal work distribution
                            best_dist = work_perf.loc[
                                work_perf["execution_time_ms"].idxmin()
                            ]
                            f.write(
                                f"\n  Optimal work distribution: "
                                f"Inside: {best_dist['inside_work']} ({best_dist['inside_percentage']:.1f}%), "
                                f"Outside: {best_dist['outside_work']} ({100 - best_dist['inside_percentage']:.1f}%)\n\n"
                            )

            # Add detailed tables
            f.write("Detailed Results:\n")
            f.write("----------------\n")
            results_df = successful_results[
                [
                    "threads",
                    "iterations",
                    "inside_work",
                    "outside_work",
                    "inside_percentage",
                    "execution_time_ms",
                ]
            ]
            for detailed_results in results_df.to_string(index=False).split("\n"):
                f.write(f"{detailed_results}\n")

        print(f"\nResults saved to {csv_filename}")
        print(f"Detailed report saved to {report_filename}")
        print(f"Test log saved to {log_filename}")
    else:
        print("No successful test results to analyze.")
        print(f"Test log saved to {log_filename}")


if __name__ == "__main__":
    main()
