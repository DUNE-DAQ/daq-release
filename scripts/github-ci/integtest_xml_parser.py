import os
import sys
import re
import argparse
import html
import xml.etree.ElementTree as ET
from pathlib import Path
from dataclasses import dataclass, field

@dataclass
class TestCaseResult:
    name: str
    status: str
    failure_message: str = ''

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

class JUnitXMLParser:
    def __init__(self, input_directory: str='', input_file: str=''):
        self.input_directory = Path(input_directory) if Path(input_directory).is_dir() else None
        self.input_file = Path(input_file) if Path(input_file).is_file() else None
        self.test_results = []
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

        for testcase in root.findall(".//testcase"):
            test_name = testcase.get("name").split("[")[0]
            result = "passed"
            failure_message = None

            if (failure_element := testcase.find("failure")) is not None:
                result = "failed"
                failure_message = html.unescape(failure_element.text.strip()) if failure_element.text else "No message provided"
            elif testcase.find("error") is not None:
                result = "error"
            elif testcase.find("skipped") is not None:
                result = "skipped"

            self.test_results.append({
                "package_name": package_name,
                "pytest_name": pytest_name,
                "test_name": test_name,
                "result": result,
                "failure_message": failure_message
            })

    def summarize_failures(self):
        failure_summary = ''
        failed_line_pattern = r"\n>\s+(.*?)\n"

        for result in self.test_results:
            if not result.get('failure_message'):
                continue
            failure_summary += f"\n\t\t  *{result['test_name']}* failed"
            failed_line_match = re.findall(failed_line_pattern, result['failure_message'])
            if failed_line_match:
                failure_summary += f" while checking \n\t\t`{failed_line_match[0]}`\n"
        return failure_summary

    def which_emoji(self, test_status):
        emoji_map = {
            'passed': ':white_check_mark:',
            'failed': ':x:',
            'skipped': ':fast_forward:',
            'error': ':warning:'
        }
        return emoji_map.get(test_status, ':question:')

    def format_markdown_row(self, testname, result):
        emoji = self.which_emoji(result)
        return f"| {testname} | {emoji} {result} |\n"

    def generate_markdown_table(self, output_filename: str="pytest_summary_table.md"):
        with open(output_filename, 'w') as f:
            for idx, result in enumerate(self.test_results):
                if not result: continue
                f.write(f"# {result['package_name']} {result['pytest_name']} Results\n")
                f.write("| Test Case | Status |\n")
                f.write("|-----------|--------|\n")
                f.writelines(self.format_markdown_row(result['pytest_name'], result['result']))

        self.prepend_totals(output_filename)

        if not Path(output_filename).is_file():
            raise FileNotFoundError(f"There was a problem writing the output markdown file: {output_filename}")

        print(f"Markdown summary generated at {output_filename}")

    def prepend_totals(self, markdown_filename="pytest_summary_table.md"):
        num_passed = 0
        num_failed = 0
        num_skipped = 0
        num_errors = 0
        total_tests = 0
        if not Path(markdown_filename).is_file():
            raise FileNotFoundError(f"No markdown file named {markdown_filename} found.\n \
                                      Make sure to generate the markdown file using 'this.generate_markdown_table()'.")
        with open(markdown_filename, 'r') as ifile:
            original_lines = ifile.readlines()
            for line in original_lines:
                print(line)
                if 'passed' in line:
                    num_passed += 1
                    total_tests += 1
                elif 'failed' in line:
                    num_failed += 1
                    total_tests += 1
                elif 'skipped' in line:
                    num_skipped += 1
                    total_tests += 1
                elif 'error' in line:
                    num_errors += 1
                    total_tests += 1

        print('Passed:', num_passed)
        print('Skipped:', num_skipped)
        print('Failed:', num_failed)
        print('Errors:', num_errors)

        summary = f"""There were {total_tests} total tests run. 
           {num_passed} passed {self.which_emoji('passed')}, 
           {num_failed} failed {self.which_emoji('failed')}, 
           {num_skipped} were skipped {self.which_emoji('skipped')}, and
           {num_errors} had errors {self.which_emoji('error')} which prevented the test from completing.\n"""
        new_lines = [summary] + original_lines
        with open(markdown_filename, 'w') as ofile:
            ofile.writelines(new_lines)

    def parse_integration_test_summary(self, pytest_summary):
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
    summary = integtest_parser.parse_integration_test_summary('pytest_summary_table.md')
    print('SUMMARY:', summary)


if __name__ == "__main__":
    main()
