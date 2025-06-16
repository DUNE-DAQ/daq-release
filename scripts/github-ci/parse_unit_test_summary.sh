#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 path/to/unit_test_summary.log"
  exit 1
fi

log_summary_file="$1"

if [[ ! -f "$log_summary_file" ]]; then
  echo "Error: File '$log_summary_file' not found."
  exit 1
fi

echo "# Unit Test Report"

table_header_line="| Test | Status |
| --- | --- |"

previous_package=""
markdown_content=""

while IFS= read -r line; do
  # Strip out ANSI color codes
  cleaned_line=$(echo "$line" | sed -E 's/\x1B\[[0-9;]*[a-zA-Z]//g')

  # Determine package name
  if [[ "$cleaned_line" == *"No unit tests"* ]]; then
    this_package=$(echo "$cleaned_line" | cut -d ' ' -f 9)
  else
    this_package=$(echo "$cleaned_line" | cut -d '/' -f 2)
  fi

  # Extract test name
  test_name=$(echo "$line" | cut -d '/' -f 4 | cut -d '.' -f 1)

  # Add package section if it changes
  if [[ "$this_package" != "$previous_package" ]]; then
    markdown_content+="
## $this_package
$table_header_line"
    previous_package="$this_package"
  fi

  # Determine test result
  if echo "$line" | grep -q "SUCCESS"; then
    markdown_content+="
| $test_name | :white_check_mark: Passed |"
  elif echo "$line" | grep -q "FAILURE"; then
    markdown_content+="
| $test_name | :x: Failed |"
  else
    markdown_content+="
:construction: Unit tests have not been written for $this_package"
  fi

done < "$log_summary_file"

echo "$markdown_content"
