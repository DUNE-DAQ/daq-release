import os
import argparse
from pathlib import Path
import xml.etree.ElementTree as ET

def get_xml_files(directory, pattern):
    path = Path(directory)
    return path.rglob(pattern)

def get_test_name(file_path):
    file_name = os.path.basename(file_path)
    # Results file names should look like minimal_system_quick_test_results.xml
    return file_name.replace('_results.xml', '')

def parse_junit_xml(file_path):
    tree = ET.parse(file_path)
    root = tree.getroot()

    test_suite_name = get_test_name(file_path)
    
    results = []
    for testcase in root.findall(".//testcase"):
        test_name = testcase.get("name").split("[")[0]
        result = "passed"
        
        if testcase.find("failure") is not None:
            result = "failed"
        elif testcase.find("error") is not None:
            result = "error"
        elif testcase.find("skipped") is not None:
            result = "skipped"

        results.append({
            "test_suite_name": test_suite_name,
            "test_name": test_name,
            "result": result
        })

    return results

def which_emoji(test_status):
    emoji_map = {
        'passed': ':white_check_mark:',
        'failed': ':x:',
        'skipped': ':grey_question:'
    }
    return emoji_map.get(test_status, ':shrug:')

def format_markdown_row(test):
    emoji = which_emoji(test['result'])
    return f"| {test['test_name']} | {emoji} {test['result']} |\n"

def generate_markdown_table(results, output_filename):
    with open(output_filename, 'w') as f:
        for idx, result in enumerate(results):
            f.write(f"# {result[0]['test_suite_name']} Results\n")
            f.write("| Test Case | Status |\n")
            f.write("|-----------|--------|\n")
            f.writelines(format_markdown_row(test) for test in result)
    
    if not os.path.exists(output_filename):
        raise FileNotFoundError(f"There was a problem writing the output markdown file: {output_filename}")

    print(f"Markdown summary generated at {output_filename}")
    return

def prepend_test_summary(markdown_file):
    num_passed = 0
    num_failed = 0
    num_skipped = 0
    total_tests = 0
    with open(markdown_file, 'r') as ifile:
        original_lines = ifile.readlines()
        #print(original_lines)
        for line in original_lines:
            print(line)
            #print(line)
            if 'passed' in line:
                num_passed += 1
                total_tests += 1
            elif 'failed' in line:
                num_failed += 1
                total_tests += 1
            elif 'skipped' in line:
                num_skipped += 1
                total_tests += 1

    print('Passed:', num_passed)
    print('Failed:', num_failed)

    summary = f"{num_passed} passed and {num_failed} failed of {total_tests} total tests.\n"
    new_lines = [summary] + original_lines
    with open(markdown_file, 'w') as ofile:
        ofile.writelines(new_lines)

def main():
    parser = argparse.ArgumentParser(description="Parse a JUnit XML file and extract test case results.")
    parser.add_argument("--input-directory", "-d",
                        help="Path to the directory containing junit xml files.")
    parser.add_argument("--input-file", "-i",
                        help="Path to a single JUnit XML file. Cannot be used in conjunction with --input-directory")
    parser.add_argument("--output-markdown-file", "-o", 
                        default="pytest_summary_table.md",
                        help="Name of the output file containing the markdown summary table. Default: ./pytest_summary_table.md")
    
    args = parser.parse_args()

    if args.input_directory and args.input_file:
        print(f"Error: You must specify either an input directory or a specific file, not both.")
        exit(1)

    test_results = []

    if args.input_directory:
        if not os.path.isdir(args.input_directory):
            print(f"Error: {args.input_directory} is not a valid directory.")
            exit(2)

        xml_files = get_xml_files(args.input_directory, "*.xml")
        if not xml_files:
            print(f"Error: No xml files found in {args.input_directory}.")
            exit(3)

        for file in xml_files:
            test_results.append(parse_junit_xml(file))

    elif args.input_file:
        if not os.path.isfile(args.input_file):
            print(f"Error: Input file {args.input_file} is invalid.")
            exit(4)
    
        test_results.append(parse_junit_xml(args.input_file))
    else:
        print(f"Error: No input file or directory specified. Exiting...")
        exit(5)

    generate_markdown_table(test_results, args.output_markdown_file)

    prepend_test_summary(args.output_markdown_file)

if __name__ == "__main__":
    main()
