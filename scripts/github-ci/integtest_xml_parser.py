import os
import sys
import re
import argparse
import html
from pathlib import Path
import xml.etree.ElementTree as ET

class JUnitXMLParser:
    def __init__(self, input_directory=None, input_file=None, output_filename="pytest_summary_table.md"):
        self.input_directory = input_directory
        self.input_file = input_file
        self.output_filename = output_filename
        self.test_results = []

    def get_xml_files(self, directory, pattern):
        if not os.path.isdir(directory):
            raise FileNotFoundError(f"Error: {directory} is not a valid directory.")

        path = Path(directory)
        xml_files = path.rglob(pattern)
        if not xml_files:
            raise FileNotFoundError(f"Error: No xml files found in {directory}.")
        return xml_files

    def get_package_and_pytest_name_from_file(self, file_path):
        file_name = os.path.basename(file_path)
        # Junit xml files should be structured as <package_name>_<pytest_name>_results.xml
        package_and_pytest_name = file_name.replace('_results.xml', '')
        package_name, pytest_name = package_and_pytest_name.split("_", 1)
        return package_name, pytest_name

    def parse_junit_xml(self, file_path):
        tree = ET.parse(file_path)
        root = tree.getroot()

        package_name, pytest_name = self.get_package_and_pytest_name_from_file(file_path)

        results = []
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

            results.append({
                "package_name": package_name,
                "pytest_name": pytest_name,
                "test_name": test_name,
                "result": result,
                "failure_message": failure_message
            })
        return results

    def which_emoji(self, test_status):
        emoji_map = {
            'passed': ':white_check_mark:',
            'failed': ':x:',
            'skipped': ':fast_forward:',
            'error': ':warning:'
        }
        return emoji_map.get(test_status, ':shrug:')

    def format_markdown_row(self, test):
        emoji = self.which_emoji(test['result'])
        return f"| {test['test_name']} | {emoji} {test['result']} |\n"

    def generate_markdown_table(self):
        with open(self.output_filename, 'w') as f:
            for idx, result in enumerate(self.test_results):
                if not result: continue
                f.write(f"# {result[0]['package_name']} {result[0]['pytest_name']} Results\n")
                f.write("| Test Case | Status |\n")
                f.write("|-----------|--------|\n")
                f.writelines(self.format_markdown_row(test) for test in result)

        if not os.path.exists(self.output_filename):
            raise FileNotFoundError(f"There was a problem writing the output markdown file: {self.output_filename}")

        print(f"Markdown summary generated at {self.output_filename}")

    def prepend_test_summary(self):
        num_passed = 0
        num_failed = 0
        num_skipped = 0
        num_errors = 0
        total_tests = 0
        with open(self.output_filename, 'r') as ifile:
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
        with open(self.output_filename, 'w') as ofile:
            ofile.writelines(new_lines)

    def parse(self):
        if self.input_directory and self.input_file:
            print(f"Error: You must specify either an input directory or a specific file, not both.")
            exit(1)

        if self.input_directory:
            if not os.path.isdir(self.input_directory):
                raise FileNotFoundError(f"Error: {self.input_directory} is not a valid directory.")

            xml_files = self.get_xml_files(self.input_directory, "*.xml")
            for file in xml_files:
                self.test_results.append(self.parse_junit_xml(file))

        elif self.input_file:
            if not os.path.isfile(self.input_file):
                raise FileNotFoundError(f"Error: Input file {self.input_file} is invalid.")
            self.test_results.append(self.parse_junit_xml(self.input_file))

        else:
            raise RuntimeError(f"Error: No input file or directory specified. Exiting...")

        self.generate_markdown_table()
        self.prepend_test_summary()


def main():
    parser = argparse.ArgumentParser(description="Parse a JUnit XML file and extract test case results.")
    parser.add_argument("--input-directory", "-d",
                        help="Path to the directory containing junit xml files.")
    parser.add_argument("--input-file", "-i",
                        help="Path to a single JUnit XML file. Cannot be used in conjunction with --input-directory.")
    parser.add_argument("--output-markdown-file", "-o", 
                        default="pytest_summary_table.md",
                        help="Name of the output file containing the markdown summary table. Default: ./pytest_summary_table.md")

    args = parser.parse_args()

    if len(sys.argv) == 1:
        parser.print_usage()
        exit(1)

    if args.input_directory and args.input_file:
        print(f"Error: You must specify either an input directory or a specific file, not both.")
        exit(1)

    parser = JUnitXMLParser(
        input_directory=args.input_directory,
        input_file=args.input_file,
        output_filename=args.output_markdown_file
    )
    parser.parse()


if __name__ == "__main__":
    main()
