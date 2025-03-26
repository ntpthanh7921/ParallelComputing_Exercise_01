#!/usr/bin/env python3

import subprocess
import csv
import os
import logging
import argparse
from datetime import datetime


def setup_logging(log_dir, test_name):
    """
    Configure logging with custom directory and test name
    """
    # Create log directory if it doesn't exist
    os.makedirs(log_dir, exist_ok=True)

    log_filename = os.path.join(log_dir, f"{test_name}_runner.log")

    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s - %(levelname)s - %(message)s",
        handlers=[logging.FileHandler(log_filename), logging.StreamHandler()],
    )

    return log_filename


def run_performance_test(executable, num_threads, total_phases, timeout_seconds):
    """
    Run the barrier performance test with specified parameters and timeout
    """
    command = [executable, str(num_threads), str(total_phases)]
    logging.info(f"Running command: {' '.join(command)} (timeout: {timeout_seconds}s)")

    try:
        result = subprocess.run(
            command, capture_output=True, text=True, check=True, timeout=timeout_seconds
        )
        return result.stdout, None
    except subprocess.TimeoutExpired as e:
        logging.error(f"Test timed out after {timeout_seconds} seconds")
        return None, "timeout"
    except subprocess.CalledProcessError as e:
        logging.error(f"Error running test: {e}")
        logging.error(f"Command output: {e.stdout}")
        logging.error(f"Command error: {e.stderr}")
        return None, "error"


def parse_test_output(output):
    """
    Parse the output from the barrier performance test
    """
    if not output:
        return None

    results = {}

    # Parse thread count
    thread_line = [
        line for line in output.split("\n") if "threads" in line and "-" in line
    ]
    if thread_line:
        results["num_threads"] = int(thread_line[0].split("-")[1].strip().split()[0])

    # Parse total phases
    phases_line = [
        line for line in output.split("\n") if "total phases" in line and "-" in line
    ]
    if phases_line:
        results["total_phases"] = int(phases_line[0].split("-")[1].strip().split()[0])

    # Parse elapsed time
    time_line = [line for line in output.split("\n") if "Total time:" in line]
    if time_line:
        results["elapsed_seconds"] = float(
            time_line[0].split(":")[1].strip().split()[0]
        )

    # Parse performance
    perf_line = [line for line in output.split("\n") if "Performance:" in line]
    if perf_line:
        results["phases_per_second"] = float(
            perf_line[0].split(":")[1].strip().split()[0]
        )

    return results


def write_to_csv(results, csv_file):
    """
    Write test results to a CSV file
    """
    # Create directory if it doesn't exist
    os.makedirs(os.path.dirname(csv_file), exist_ok=True)

    file_exists = os.path.isfile(csv_file)

    fieldnames = [
        "timestamp",
        "test_name",
        "num_threads",
        "total_phases",
        "elapsed_seconds",
        "phases_per_second",
        "status",
    ]

    with open(csv_file, "a", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)

        if not file_exists:
            writer.writeheader()

        # Add timestamp to results
        results["timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        writer.writerow(results)


def main():
    parser = argparse.ArgumentParser(
        description="Run barrier performance tests with various configurations"
    )
    parser.add_argument(
        "executable", help="Path to the barrier performance test executable"
    )
    parser.add_argument(
        "--output-dir",
        "-d",
        default="./results",
        help="Directory to store results and logs (default: ./results)",
    )
    parser.add_argument(
        "--test-name",
        "-n",
        default="barrier_test",
        help="Name of the test (used in filenames) (default: barrier_test)",
    )
    parser.add_argument(
        "--threads",
        "-t",
        type=int,
        nargs="+",
        default=[1, 2, 4, 8, 16],
        help="Thread counts to test (default: 1 2 4 8 16)",
    )
    parser.add_argument(
        "--phases",
        "-p",
        type=int,
        nargs="+",
        default=[100000],
        help="Total phases to test (default: 100000)",
    )
    parser.add_argument(
        "--runs",
        "-r",
        type=int,
        default=3,
        help="Number of runs for each configuration (default: 3)",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=60,
        help="Timeout in seconds for each test run (default: 60)",
    )

    args = parser.parse_args()

    # Setup logging with custom directory and test name
    log_file = setup_logging(args.output_dir, args.test_name)

    # Create CSV filename based on test name
    csv_file = os.path.join(args.output_dir, f"{args.test_name}_results.csv")

    # Check if executable exists
    if not os.path.isfile(args.executable):
        logging.error(f"Executable not found: {args.executable}")
        return 1

    logging.info("Starting barrier performance test runner")
    logging.info(f"Test name: {args.test_name}")
    logging.info(f"Executable: {args.executable}")
    logging.info(f"Output directory: {args.output_dir}")
    logging.info(f"Log file: {log_file}")
    logging.info(f"CSV file: {csv_file}")
    logging.info(f"Thread counts: {args.threads}")
    logging.info(f"Phase counts: {args.phases}")
    logging.info(f"Runs per configuration: {args.runs}")
    logging.info(f"Timeout per run: {args.timeout} seconds")

    # Run tests for each configuration
    for num_threads in args.threads:
        for total_phases in args.phases:
            for run in range(1, args.runs + 1):
                logging.info(
                    f"Configuration: {num_threads} threads, {total_phases} phases (Run {run}/{args.runs})"
                )

                output, error = run_performance_test(
                    args.executable, num_threads, total_phases, args.timeout
                )

                if output:
                    results = parse_test_output(output)
                    if results:
                        # Add test name and success status to results
                        results["test_name"] = args.test_name
                        results["status"] = "success"
                        logging.info(f"Results: {results}")
                        write_to_csv(results, csv_file)
                    else:
                        logging.error("Failed to parse test output")
                        # Record failed parse
                        results = {
                            "test_name": args.test_name,
                            "num_threads": num_threads,
                            "total_phases": total_phases,
                            "status": "parse_error",
                        }
                        write_to_csv(results, csv_file)
                else:
                    # Record timeout or error
                    logging.error(f"Test execution failed with status: {error}")
                    results = {
                        "test_name": args.test_name,
                        "num_threads": num_threads,
                        "total_phases": total_phases,
                        "status": error,
                    }
                    write_to_csv(results, csv_file)

    logging.info(f"All tests completed. Results saved to {csv_file}")
    return 0


if __name__ == "__main__":
    exit(main())
