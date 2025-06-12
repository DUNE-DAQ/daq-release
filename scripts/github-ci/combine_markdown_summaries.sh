#!/bin/bash

# Usage: ./combine_test_results.sh file1.md file2.md > combined.md

# Initialize counters
total_tests=0
passed=0
failed=0
skipped=0
errored=0

# Output file header
echo -n "" > /tmp/all_tables.tmp

# Function to extract numbers from the summary
extract_counts() {
  local file="$1"
  total_tests=$((total_tests + $(grep -oP 'There were \K[0-9]+' "$file")))
  passed=$((passed + $(grep -oP '\s+\K[0-9]+(?= passed)' "$file")))
  failed=$((failed + $(grep -oP '\s+\K[0-9]+(?= failed)' "$file")))
  skipped=$((skipped + $(grep -oP '\s+\K[0-9]+(?= were skipped)' "$file")))
  errored=$((errored + $(grep -oP '\s+\K[0-9]+(?= had errors)' "$file")))
}

# Loop over all input files
for file in "$@"; do
  extract_counts "$file"

  # Extract everything from the first heading onwards (tables + section headers)
  awk '/^# /{flag=1} flag' "$file" >> /tmp/all_tables.tmp
done

# Output the combined summary
echo "There were $total_tests total tests run."
echo "           $passed passed :white_check_mark:,"
echo "           $failed failed :x:,"
echo "           $skipped were skipped :fast_forward:, and"
echo "           $errored had errors :warning: which prevented the test from completing."
echo
echo "# Combined Test Results"
echo

# Output the combined tables
cat /tmp/all_tables.tmp

# Clean up
rm /tmp/all_tables.tmp
