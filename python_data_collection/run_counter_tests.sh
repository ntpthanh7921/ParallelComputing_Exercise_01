#!/bin/bash

# Check if directory path is provided
if [ $# -ne 1 ]; then
  echo "Usage: $0 <directory_path>"
  exit 1
fi

# Directory path from command line argument
DIRECTORY="$1"

# Find all executable files in the directory
echo "Finding executable files in $DIRECTORY..."
find "$DIRECTORY" -type f -executable | while read -r executable; do
  # Get the base name of the executable for use as test-name
  base_name=$(basename "$executable")

  # Create output directory with same name as the executable
  output_dir="${base_name}"
  mkdir -p "$output_dir"

  echo "Running mutex test on: $base_name"
  echo "Output directory: $output_dir"

  # Run the mutex test with appropriate parameters
  python counter_test_many_params_with_log.py "$executable" \
    --test-name "$base_name" \
    --output-dir "$output_dir"

  echo "Test completed for $base_name"
  echo "---------------------------------"
done

echo "All tests completed!"
