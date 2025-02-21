import sys, os
import argparse
import json
import re
from integtest_xml_parser import parse_junit_xml

def get_failed_jobs(api_output_path):
    """
    Load the JSON file output from GitHub API call which checks for failed jobs and steps. 
    """
    try:
        with open(api_output_path, "r") as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

def get_xml_files(directory, pattern):
    if not os.path.isdir(directory):
        raise FileNotFoundError(f"Error: {directory} is not a valid directory.")

    path = Path(directory)
    xml_files = path.rglob(pattern)

    return xml_files

def get_workflow_status(failed_jobs):
    """
    Determine the overall workflow status.
    
    If any job or step was cancelled, this should take precedence over failure.
    Defaults to success if there were no cancellations or failures.
    """
    has_cancelled = False
    has_failure = False
    for job in failed_jobs:
        if job['conclusion'] == 'cancelled':
            has_cancelled = True
        elif job['conclusion'] == 'failure':
            has_failure = True
    
    if has_cancelled:
        return 'cancelled'
    if has_failure:
        return 'failure'
    return 'success'

def get_header(status):
    """Generate the header block based on the status."""
    status_emojis = {
        "success": ":white_check_mark:",
        "failure": ":rotating_light:",
        "cancelled": ":no_entry:",
    }
    workflow_name = os.getenv("GITHUB_WORKFLOW", "Unknown Workflow")
    return {
        "type": "header",
        "text": {
            "type": "plain_text",
            "text": f"{status_emojis.get(status, ':grey_question:')} {status.capitalize()}: {workflow_name} {status_emojis.get(status, ':grey_question:')}",
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
    """Find junit xml file that matches the job name."""
    for file in xml_files:
        if test_name in str(file):
            return file
    return None

def get_integration_test_failure(test_name, xml_files):
    """Get the primary failure type from the junit xml file."""
    xml_file = find_matching_file(test_name, xml_files)
    if not xml_file:
        print('WARNING: No matching xml file found for', test_name)
        return "Unknown failure: could not find results.xml file for this test."

    failure = "Unknown failure"
    results = parse_junit_xml(xml_file)
    # Lines that failed will be preceeded by a newline followed by ">" and
    # some number of whitespace characters; search between this and the next
    # newline to see what the failure was.
    failed_line_pattern = r"\n>\s+(.*?)\n"
    failure_summary = ''
    for result in results:
        if not result.get('failure_message'): continue
        failure_summary += f"\n\t\t  *{result['test_name']}* failed"
        failed_line_match = re.findall(failed_line_pattern, result['failure_message'])
        if failed_line_match:
            failure_summary += f" while checking \n\t\t`{failed_line_match[0]}`\n" 
    return failure_summary

def get_failed_jobs_section(failed_jobs, xml_files):
    """Generate the section block that lists failed jobs and steps."""
    if not failed_jobs:
        return None

    failed_jobs_text = "*Failed jobs and steps:*\n"
    for job in failed_jobs:
        # Jobs may be skipped or have not yet run
        if not job['conclusion'] == 'failure': continue
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

def get_pytest_log_section(pytest_log_dir):
    """Generate a section showing where the pytest logs are stored."""
    return {
        "type": "section",
        "text": {
            "type": "mrkdwn",
            "text": f"The pytest logs will be stored for the next 7 days at: `daq.fnal.gov:{pytest_log_dir}`.\n"
                     "You can also download the logs from the \"Full report\" link above (scroll to the bottom)."
        }
    }

def generate_payload(failed_jobs, xml_files=[], pytest_log_dir=None):
    """
    Top-level payload generator function that determines which payload sections to
    write based on inputs. The message is written such that it can be parsed by
    Slack's BlockKit format.
    """
    slack_payload = {"blocks": []}

    workflow_status = get_workflow_status(failed_jobs)
    slack_payload["blocks"].append(get_header(workflow_status))

    slack_payload["blocks"].append(get_report_section())

    failed_jobs_section = ''
    if workflow_status == 'failure':
        failed_jobs_section = get_failed_jobs_section(failed_jobs, xml_files)
    if failed_jobs_section:
        slack_payload["blocks"].append(failed_jobs_section)

    if pytest_log_dir:
        slack_payload["blocks"].append(get_pytest_log_section(pytest_log_dir))
    
    return slack_payload

def write_payload_to_file(payload, file_name='slack_payload.json'):
    """Write the Slack payload to a file which can be seen by slack-github-action."""
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
    parser.add_argument("--junit-xml-dir", nargs="?", default=None,
                        help="Optional directory containing junit xml files output by pytests.")
    parser.add_argument("--pytest-log-dir", nargs="?", default=None,
                        help="Optional directory containing the full pytest output.")
    args = parser.parse_args()
    if len(sys.argv) == 1:
        parser.print_usage()

    failed_jobs = get_failed_jobs(args.api_output)

    # Nightly integration tests store their output in junit xml files
    xml_files = []
    if args.junit_xml_dir:
        xml_files = get_xml_files(args.junit_xml_dir, "*_results.xml")

    payload = generate_payload(failed_jobs, xml_files, args.pytest_log_dir)
    write_payload_to_file(payload)
    
if __name__ == "__main__":
    main()