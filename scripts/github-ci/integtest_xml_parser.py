from __future__ import annotations
import sys
import re
import argparse
import html
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Optional
from integration_test_summary import IntegrationTestSummary, PytestResult, TestCaseResult

class JUnitXMLParser:
    def __init__(self, input_directory: Optional[str]=None, input_file: Optional[str]=None):
        self.input_directory: Optional[Path] = None
        self.input_file: Optional[Path] = None
        self.summary = IntegrationTestSummary()

        if input_directory:
            self.input_directory = Path(input_directory)
            if not self.input_directory.is_dir():
                raise FileNotFoundError(f"Input directory {input_directory} doesn't exist or isn't a directory.")

        if input_file:
            self.input_file = Path(input_file)
            if not self.input_file.is_file():
                raise FileNotFoundError(f"Input file {input_file} doesn't exist or isn't a file.")

        if self.input_file and self.input_directory:
            raise ValueError("Specify one of --input-directory or --input-file, not both.")

        if not input_file and not input_directory:
            # Before throwing, write empty test results so that downstream workflows don't fail
            self.summary.to_json()
            raise ValueError("No valid input file or directory specified.")

    def get_xml_files(self, pattern="*.xml"):
        xml_files = list(self.input_directory.rglob(pattern))
        if not xml_files:
            raise FileNotFoundError(f"Error: No xml files found in {self.input_directory}.")
        return xml_files

    # JUnit xml file names should be structured as <package_name>_<pytest_name>_results.xml
    def get_package_name(self, file) -> str:
        return file.stem.split('_')[0]

    def get_pytest_name(self, file) -> str:
        return file.stem.replace('_results', '').split('_', 1)[1]

    def parse_xml_file(self, file_path: str) -> None:
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
    parser = argparse.ArgumentParser(description="Parse JUnit XML file(s) and extract test case results.")
    parser.add_argument("--input-directory", "-d", type=str, default='',
                        help="Path to the directory containing junit xml files.")
    parser.add_argument("--input-file", "-i", type=str, default='',
                        help="Path to a single JUnit XML file. Cannot be used in conjunction with --input-directory.")

    args = parser.parse_args()

    integtest_parser = JUnitXMLParser(
        input_directory=args.input_directory,
        input_file=args.input_file,
    )

    if integtest_parser.input_directory:
        try:
            xml_files = integtest_parser.get_xml_files()
        except FileNotFoundError:
            print(f'WARNING: No JUnit XML files found in {integtest_parser.input_directory}; '
                  f'test results will be empty.')
            xml_files = []
        for file in xml_files:
            integtest_parser.parse_xml_file(file)

    elif integtest_parser.input_file:
        integtest_parser.parse_xml_file(integtest_parser.input_file)

    integtest_parser.summary.to_markdown()
    integtest_parser.summary.to_json()

if __name__ == "__main__":
    main()
