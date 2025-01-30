import sys
import json
import argparse
import re
from integtest_xml_parser import get_xml_files, parse_junit_xml

def get_failed_jobs(api_output_path):
    """
    Load the JSON file output from GitHub API call which checks for failed jobs and steps. 
    """
    try:
        with open(api_output_path, "r") as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

def get_header(status):
    """Generate the header block based on the status."""
    status_emojis = {
        "success": ":white_check_mark:",
        "failure": ":rotating_light:"
    }
    return {
        "type": "header",
        "text": {
            "type": "plain_text",
            "text": f"{status_emojis.get(status, ':grey_question:')} {status.capitalize()}: GitHub Workflow {status_emojis.get(status, ':grey_question:')}",
            "emoji": True
        }
    }

def get_report_section():
    """Generate the section block with the link to full GitHub Actions output."""
    return {
        "type": "section",
        "text": {
            "type": "mrkdwn",
            "text": "Full report: <https://github.com/${{ github.payload.repository.full_name }}/actions/runs/${{ github.runId }}|link>."
        }
    }

def find_matching_file(test_name, xml_files):
    for file in xml_files:
        if test_name in str(file):
            return file
    return None

def summarize_integration_test_failure(failure_message):
    # Get the test name
    match = re.search(r"def (\w+)\(", failure_message)
    test_name = match.group(1) if match else "Unknown test"

    # Get the type of error, e.g., AssertionError
    error_pattern = r"([A-Za-z]+Error):"
    error_match = re.search(error_pattern, failure_message)
    error_type = error_match.group(1) if error_match.group(1) else "Unknown error type"

    # Get the function that caused the failure
    function_match = re.search(r"<function (\w+) at 0x[0-9a-fA-F]+>", failure_message)
    failed_function = function_match.group(1) if function_match.group(1) else "Unknown function"

    summary = f"\t\t   *{test_name}* failed with: {error_type} in function {failed_function}\n"

    return summary

def get_integration_test_failure(test_name, xml_files):
    xml_file = find_matching_file(test_name, xml_files)
    if not xml_file:
        raise FileNotFoundError('No matching xml file found for', test_name)

    results = parse_junit_xml(xml_file)
    for result in results:
        if result.get('failure_message'):
            failure = summarize_integration_test_failure(result['failure_message'])

    return failure

def get_failed_jobs_section(failed_jobs, xml_files):
    """Generate the section block that lists failed jobs and steps."""
    if not failed_jobs:
        return None

    failed_jobs_text = "*Failed jobs and steps:*\n"
    for job in failed_jobs:
        failed_jobs_text += f"- *{job['job']}*\n"
        for step in job['steps']:
            failed_jobs_text += f"\t:x: *{step['name']}*\n"
            if 'integration_tests' in job['job']:
                test_name = job['job'].split()[1].strip("()")
                failed_jobs_text += get_integration_test_failure(test_name, xml_files)

    return {
        "type": "section",
        "text": {
            "type": "mrkdwn",
            "text": failed_jobs_text
        }
    }

def generate_payload(failed_jobs, xml_files=[]):
    """
    Top-level payload generator function that determines which payload sections to
    write based on inputs. The message is written such that it can be parsed by
    Slack's BlockKit format
    """
    slack_payload = {"blocks": []}

    workflow_status = ('failure' if failed_jobs else 'success')
    slack_payload["blocks"].append(get_header(workflow_status))

    slack_payload["blocks"].append(get_report_section())

    failed_jobs_section = get_failed_jobs_section(failed_jobs, xml_files)
    if failed_jobs_section:
        slack_payload["blocks"].append(failed_jobs_section)
    
    return slack_payload

def write_payload_to_file(payload, file_name='slack_payload.json'):
    try:
        with open(file_name, "w") as file:
            json.dump(payload, file)
        print(f"Payload written to {file_name}")
    except Exception as e:
        print(f"Failed to write payload to file: {e}")

def main():
    parser = argparse.ArgumentParser(description="Parse a workflow summary and generate a Slack message payload.")
    parser.add_argument("--api-output", required=True,
                        help="json file containing a summary of failed jobs, obtained from GitHub API call.")
    parser.add_argument("--junit-xml-dir", default="",
                        help="Optional directory containing junit xml files output by pytests.")
    args = parser.parse_args()
    if len(sys.argv) == 1:
        parser.print_usage()

    failed_jobs = get_failed_jobs(args.api_output)

    # Nightly integration tests store their output in junit xml files
    xml_files = []
    if args.junit_xml_dir:
        xml_files = get_xml_files(args.junit_xml_dir, "*_results.xml")

    payload = generate_payload(failed_jobs, xml_files)
    write_payload_to_file(payload)
    
if __name__ == "__main__":
    main()