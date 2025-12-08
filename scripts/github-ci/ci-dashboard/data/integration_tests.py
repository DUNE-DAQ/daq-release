import re
import json
from pathlib import Path
from dataclasses import dataclass, field

@dataclass
class TestCaseResult:
    name: str
    status: str

@dataclass
class IntegrationTestSummary:
    passed: int = 0
    failed: int = 0
    skipped: int = 0
    errors: int = 0
    total: int = 0
    repo_results: dict[str, list[TestCaseResult]] = field(default_factory=dict)

    def to_dict(self):
        return {
            "passed": self.passed,
            "failed": self.failed,
            "skipped": self.skipped,
            "errors": self.errors,
            "total": self.total,
            "repo_results": self.repo_results
        }

def parse_integration_test_summary(pytest_summary):
    summary = IntegrationTestSummary()

    with open(pytest_summary, "r") as f:
        num_header_lines = 5
        lines = [line.strip() for line in f if line.strip()]
        header_lines = lines[:num_header_lines]
        results_lines = lines[num_header_lines:]

    numbers = [int(re.search(r'\d+', line).group()) for line in header_lines]
    total, passed, failed, skipped, errors = numbers
    summary.total = total
    summary.passed = passed
    summary.failed = failed
    summary.skipped = skipped
    summary.errors = errors
    # Parse test results line by line
    i = 0
    while i < len(results_lines):
        line = results_lines[i]
        if line.startswith("#"):
            # Each section represents a new package
            section_title = re.sub(r"^#+ ", "", line)
            #summary["results"][section_title] = []
            summary.repo_results[section_title] = []
            i += 3  # Skip table header and separator

            # Read test rows until next heading or end of list
            while i < len(results_lines) and not results_lines[i].startswith("#"):
                row = results_lines[i]
                if "|" in row:
                    columns = [c.strip() for c in row.strip("|").split("|")]
                    if len(columns) == 2:
                        test_name, status_col = columns
                        status = "passed" if "passed" in status_col else "failed" if "failed" in status_col else "skipped" if "skipped" in status_col else "error"
                        #summary["results"][section_title].append({
                        #summary.repo_results[section_title].append({
                        #    "name": test_name,
                        #    "status": status
                        #})
                        test_result = TestCaseResult(name=test_name, status=status)
                        summary.repo_results[section_title].append(test_result)
                i += 1
        else:
            i += 1

    return summary