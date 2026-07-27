import re
from pathlib import Path
from typing import Optional
from dataclasses import dataclass, field

@dataclass
class UnitTestReport:
    # Map repo_name to a list of tuples, where each tuple is ('test_name', 'status')
    repo_results: dict[str, list[tuple[Optional[str], str]]] = field(default_factory=dict)

    def _count_status(self, status: str) -> int:
        return sum(1 for tests in self.repo_results.values() for _, s in tests if s == status)

    @property
    def passed(self) -> int:
        return self._count_status("Passed")

    @property
    def failed(self) -> int:
        return self._count_status("Failed")

    @property
    def other(self) -> int:
        return sum(1 for tests in self.repo_results.values()
                   for _, s in tests if s not in ("Passed", "Failed", "NoTests"))

    @property
    def no_tests(self) -> list[str]:
        return [repo for repo, tests in self.repo_results.items()
                if any(s == "NoTests" for _, s in tests)]

    @property
    def total(self) -> int:
        """Total passing percentage is calculated relative to the tests
        which actually ran, not skipped or errored tests."""
        return self.passed + self.failed

    def parse(self, log_path):
        """Parse unit test summary and store as a dictionary."""
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

                if this_package not in self.repo_results:
                    self.repo_results[this_package] = []

                if "SUCCESS" in clean_line and test_name:
                    self.repo_results[this_package].append((test_name, "Passed"))
                elif "FAILURE" in clean_line and test_name:
                    self.repo_results[this_package].append((test_name, "Failed"))
                elif test_name is None:
                    self.repo_results[this_package].append((None, "NoTests"))
        
    def to_dict(self):
        return {
            "passed": self.passed,
            "failed": self.failed,
            "total": self.total,
            "other": self.other,
            "no_tests": self.no_tests,
            "repo_results": self.repo_results
        }

def strip_ansi(line):
    """Strip color coding from unit test summary log."""
    return re.sub(r"\x1B\[[0-9;]*[a-zA-Z]", "", line)

if __name__ == "__main__":
    pass