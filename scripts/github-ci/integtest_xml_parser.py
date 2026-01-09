import os
import sys
import re
import argparse
import html
import xml.etree.ElementTree as ET
from pathlib import Path
from textwrap import dedent
from dataclasses import dataclass, field

sys.path.append(str(Path(__file__).resolve().parents[2]))
from parsers.junit_xml import JUnitXMLParser

def main():
    parser = argparse.ArgumentParser(description="Parse a JUnit XML file and extract test case results.")
    parser.add_argument("--input-directory", "-d", type=str, default='',
                        help="Path to the directory containing junit xml files.")
    parser.add_argument("--input-file", "-i", type=str, default='',
                        help="Path to a single JUnit XML file. Cannot be used in conjunction with --input-directory.")
    parser.add_argument("--to-json", "-j", action='store_true',
                        help="Write summary data to a json file.")

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

    integtest_parser.summary.to_markdown()

    if args.to_json:
        integtest_parser.summary.to_json()

if __name__ == "__main__":
    main()
