from __future__ import annotations
import re
import json
from pathlib import Path
from textwrap import dedent
from dataclasses import dataclass, field, asdict

@dataclass
class TestCaseResult:
    case_name: str
    status: str
    failure_message: str = ''

    @classmethod
    def from_dict(cls, d: dict) -> TestCaseResult:
        return cls(**d)

    def summarize_failure(self) -> str:
        if not self.failure_message:
            return

        failure_summary = ''
        failed_line_pattern = r"\n>\s+(.*?)\n"

        failure_summary += f"\n\t\t  *{self.case_name}* failed"
        failed_line_match = re.findall(failed_line_pattern, self.failure_message)
        if failed_line_match:
            failure_summary += f" while checking \n\t\t`{failed_line_match[0]}`\n"
        return failure_summary

@dataclass
class PytestResult:
    repo_name: str
    test_name: str
    testcase_results: list[TestCaseResult] = field(default_factory=list)

    @property
    def passed(self) -> int:
        return sum(tc.status == "passed" for tc in self.testcase_results)

    @property
    def failed(self) -> int:
        return sum(tc.status == "failed" for tc in self.testcase_results)

    @property
    def skipped(self) -> int:
        return sum(tc.status == "skipped" for tc in self.testcase_results)

    @property
    def errors(self) -> int:
        return sum(tc.status == "error" for tc in self.testcase_results)

    @classmethod
    def from_dict(cls, d: dict) -> PytestResult:
        cases = [TestCaseResult.from_dict(case) for case in d["testcase_results"]]
        return cls(
            repo_name=d["repo_name"],
            test_name=d["test_name"],
            testcase_results=cases,
        )

@dataclass
class IntegrationTestSummary:
    pytest_results: list[PytestResult] = field(default_factory=list)

    EMOJI_MAP = {
        'passed': ':white_check_mark:',
        'failed': ':x:',
        'skipped': ':fast_forward:',
        'error': ':warning:'
        'all_passed': ':tada:'
    }

    def which_emoji(self, test_status: str) -> str:
        return self.EMOJI_MAP.get(test_status, ':question:')

    @property
    def totals(self) -> dict[str, int]:
        passed = sum(pr.passed for pr in self.pytest_results)
        failed = sum(pr.failed for pr in self.pytest_results)
        skipped = sum(pr.skipped for pr in self.pytest_results)
        errors = sum(pr.errors for pr in self.pytest_results)
        return {
            "passed": passed,
            "failed": failed,
            "skipped": skipped,
            "errors": errors,
            "total_run": passed + failed
        }

    @property
    def summary_text(self) -> str:
        if (self.totals['passed'] > 0 and
            self.totals['failed'] == 0 and
            self.totals['skipped'] == 0 and
            self.totals['errors'] == 0
        ):
            return f"All tests passed {self.which_emoji('all_passed')}"

        return dedent(f"""\
            {self.totals['total_run']} total tests run. 
            {self.totals['passed']} passed {self.which_emoji('passed')}, 
            {self.totals['failed']} failed {self.which_emoji('failed')}, 
            {self.totals['skipped']} were skipped {self.which_emoji('skipped')}, and
            {self.totals['errors']} had errors {self.which_emoji('error')} which prevented the test from completing.\n"""
        )

    @classmethod
    def from_dict(cls, data: dict) -> IntegrationTestSummary:
        pytest_results = [
            PytestResult.from_dict(test)
            for repo_tests in data["pytest_results"].values()
            for test in repo_tests
        ]
        return cls(pytest_results=pytest_results)

    @classmethod
    def from_json_file(cls, path: str | Path) -> IntegrationTestSummary:
        path = Path(path)
        data = json.loads(path.read_text())
        return cls.from_dict(data)

    def to_dict(self) -> dict:
        grouped: dict[str, list[dict]] = {}
        for pr in self.pytest_results:
            if pr.repo_name not in grouped:
                grouped[pr.repo_name] = []
            grouped[pr.repo_name].append(asdict(pr))

        return {
            "totals": {
                "passed": self.totals['passed'],
                "failed": self.totals['failed'],
                "skipped": self.totals['skipped'],
                "errors": self.totals['errors'],
                "total_run": self.totals['total_run'],
            },
            "summary_text": self.summary_text,
            "pytest_results": grouped
        }

    def to_json(self, path: str="pytest_summary.json"):
        json_path = Path(path)
        json_path.write_text(json.dumps(self.to_dict(), indent=2))
        print(f"json summary generated at {path}")

    def to_markdown(self, output_filename: str="pytest_summary_table.md"):
        def which_emoji(test_status: str) -> str:
            emoji_map = {
                'passed': ':white_check_mark:',
                'failed': ':x:',
                'skipped': ':fast_forward:',
                'error': ':warning:'
            }
            return emoji_map.get(test_status, ':question:')

        def format_markdown_row(testname: str, result: str) -> str:
            emoji = which_emoji(result)
            return f"| {testname} | {emoji} {result} |\n"

        with open(output_filename, 'w') as f:
            f.write(self.summary_text)

            for idx, pytest_result in enumerate(self.pytest_results):
                f.write(f"# {pytest_result.repo_name} {pytest_result.test_name} Results\n")
                f.write("| Test Case | Status |\n")
                f.write("|-----------|--------|\n")
                for ic, case in enumerate(pytest_result.testcase_results):
                    f.writelines(format_markdown_row(case.case_name, case.status))

        if not Path(output_filename).is_file():
            raise FileNotFoundError(f"There was a problem writing the output markdown file: {output_filename}")

        print(f"Markdown summary generated at {output_filename}")

    def combine(self, additional_summary: IntegrationTestSummary) -> None:
        self.pytest_results.extend(additional_summary.pytest_results)

    @classmethod
    def combined(cls, *summaries: IntegrationTestSummary) -> IntegrationTestSummary:
        merged = cls()
        for s in summaries:
            merged.combine(s)
        return merged

