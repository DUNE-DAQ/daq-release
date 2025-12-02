import re
import json
from pathlib import Path
from dataclasses import dataclass, field

@dataclass
class UnitTestSummary:
    passed: int = 0
    failed: int = 0
    other: int = 0
    no_tests: list[str] = field(default_factory=list)
    # Map repo_name to a list of tuples, where each tuple is ('test_name', 'status')
    repo_results: dict[str, list[tuple[str, str]]] = field(default_factory=dict)

    @property
    def total(self) -> int:
        """Total passing percentage is calculated relative to the tests
        which actually ran, not skipped or errored tests."""
        return self.passed + self.failed

    def get_totals(self):
        for repo_name, tests in self.repo_results.items():
            for _, status in tests:
                if status == "Passed":
                    self.passed += 1
                elif status == "Failed":
                    self.failed += 1
                elif status == 'NoTests':
                    if repo_name not in self.no_tests:
                        self.no_tests.append(repo_name)
                else:
                    self.other += 1
        
    def to_dict(self):
        return {
            "passed": self.passed,
            "failed": self.failed,
            "other": self.other,
            "no_tests": self.no_tests,
            "repo_results": self.repo_results
        }

def strip_ansi(line):
    """Strip color coding from unit test summary log."""
    return re.sub(r"\x1B\[[0-9;]*[a-zA-Z]", "", line)

def parse_unit_test_summary(log_path):
    """Parse unit test summary and store as a dictionary."""
    summary = UnitTestSummary()
    current_package = None

    with open(log_path, "r") as f:
        for line in f:
            clean_line = strip_ansi(line.strip())

            if "No unit tests" in clean_line:
                this_package = clean_line.split()[8]
                test_name = None
            else:
                parts = clean_line.split("/")
                this_package = parts[1]
                test_name = parts[3].split(".")[0]

            if this_package not in summary.repo_results:
                summary.repo_results[this_package] = []

            if "SUCCESS" in clean_line:
                summary.repo_results[this_package].append((test_name, "Passed"))
            elif "FAILURE" in clean_line:
                summary.repo_results[this_package].append((test_name, "Failed"))
            elif test_name is None:
                summary.repo_results[this_package].append((test_name, "NoTests"))

    summary.get_totals()

    return summary

if __name__ == "__main__":
    pass