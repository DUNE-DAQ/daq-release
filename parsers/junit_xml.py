import os
import sys
import re
import argparse
import html
import json
import xml.etree.ElementTree as ET
from pathlib import Path
from textwrap import dedent
from dataclasses import dataclass, field, asdict
from collections import defaultdict

@dataclass
class TestCaseResult:
    case_name: str
    status: str
    failure_message: str = ''

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


@dataclass
class IntegrationTestSummary:
    pytest_results: list[PytestResult] = field(default_factory=list)

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
            "pytest_results": grouped
        }

    def to_json(self, path: str="pytest_summary.json"):
        json_path = Path(path)
        json_path.write_text(json.dumps(self.to_dict(), indent=2))

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
            summary_string = dedent(f"""\
                {self.totals['total_run']} total tests run. 
                {self.totals['passed']} passed {which_emoji('passed')}, 
                {self.totals['failed']} failed {which_emoji('failed')}, 
                {self.totals['skipped']} were skipped {which_emoji('skipped')}, and
                {self.totals['errors']} had errors {which_emoji('error')} which prevented the test from completing.\n""")
            f.write(summary_string)

            for idx, pytest_result in enumerate(self.pytest_results):
                f.write(f"# {pytest_result.repo_name} {pytest_result.test_name} Results\n")
                f.write("| Test Case | Status |\n")
                f.write("|-----------|--------|\n")
                for ic, case in enumerate(pytest_result.testcase_results):
                    f.writelines(format_markdown_row(case.case_name, case.status))

        if not Path(output_filename).is_file():
            raise FileNotFoundError(f"There was a problem writing the output markdown file: {output_filename}")

        print(f"Markdown summary generated at {output_filename}")

class JUnitXMLParser:
    def __init__(self, input_directory: str='', input_file: str=''):
        self.input_directory = Path(input_directory) if Path(input_directory).is_dir() else None
        self.input_file = Path(input_file) if Path(input_file).is_file() else None
        self.summary = IntegrationTestSummary()

        if not self.input_directory and not self.input_file:
            raise FileNotFoundError("No valid input file or directory specified.")

    def get_xml_files(self, pattern="*.xml"):
        xml_files = self.input_directory.rglob(pattern)
        if not xml_files:
            raise FileNotFoundError(f"Error: No xml files found in {self.input_directory}.")
        return xml_files

    # Junit xml file names should be structured as <package_name>_<pytest_name>_results.xml
    def get_package_name(self, file):
        return file.stem.split('_')[0]

    def get_pytest_name(self, file):
        return file.stem.replace('_results', '').split('_', 1)[1]

    def parse_xml_file(self, file_path):
        tree = ET.parse(file_path)
        root = tree.getroot()

        package_name = self.get_package_name(file_path)
        pytest_name  = self.get_pytest_name(file_path)

        pytest_result = PytestResult(
            repo_name=package_name,
            test_name=pytest_name
        )

        for testcase in root.findall(".//testcase"):
            test_name = testcase.get("name").split("[")[0]
            result = "passed"
            failure_message = ""

            if (failure_element := testcase.find("failure")) is not None:
                result = "failed"
                failure_message = html.unescape(failure_element.text.strip()) if failure_element.text else "No message provided"
            elif testcase.find("error") is not None:
                result = "error"
            elif testcase.find("skipped") is not None:
                result = "skipped"
            
            testcase_result = TestCaseResult(
                case_name=test_name,
                status=result,
                failure_message=failure_message
            )

            pytest_result.testcase_results.append(testcase_result)

        self.summary.pytest_results.append(pytest_result)

def main():
    parser = argparse.ArgumentParser(description="Parse a JUnit XML file and extract test case results.")
    parser.add_argument("--input-directory", "-d", type=str, default='',
                        help="Path to the directory containing junit xml files.")
    parser.add_argument("--input-file", "-i", type=str, default='',
                        help="Path to a single JUnit XML file. Cannot be used in conjunction with --input-directory.")

    args = parser.parse_args()

    if len(sys.argv) == 1:
        parser.print_usage()
        exit(1)

    integtest_parser = JUnitXMLParser(
        input_directory=args.input_directory,
        input_file=args.input_file,
    )

    if integtest_parser.input_directory:
        xml_files = integtest_parser.get_xml_files()
        for file in xml_files:
            integtest_parser.parse_xml_file(file)

    elif integtest_parser.input_file:
        integtest_parser.parse_xml_file(integtest_parser.input_file)

    integtest_parser.generate_markdown_table()
    #integtest_parser.prepend_totals()
    #summary = integtest_parser.parse_integration_test_summary('pytest_summary_table.md')
    print('SUMMARY:', integtest_parser.summary)


if __name__ == "__main__":
    main()
