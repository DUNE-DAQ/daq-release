import sys
import json
import argparse
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
    """Generate the section block with the full report link."""
    return {
        "type": "section",
        "text": {
            "type": "mrkdwn",
            "text": "Full report: <https://github.com/${{ github.payload.repository.full_name }}/actions/runs/${{ github.runId }}|link>."
        }
    }

def get_failed_jobs_section(failed_jobs):
    if not failed_jobs:
        return None
    failed_jobs_text = "*Failed jobs and steps:*\n"
    for job in failed_jobs:
        failed_jobs_text += f"- *{job['job']}*\n"
        for step in job['steps']:
            failed_jobs_text += f"\t:x: *{step['name']}*\n"

    return {
        "type": "section",
        "text": {
            "type": "mrkdwn",
            "text": failed_jobs_text
        }
    }

def generate_payload(failed_jobs, xml_files=[]):
    """
    Top-level payload generator function that determines which payload to generate based on 
    the presence of job failures. The failure information is parsed to generate a message 
    payload in Slack's BlockKit format.  
    """
    slack_payload = {"blocks": []}

    workflow_status = ('failure' if failed_jobs else 'success')
    slack_payload["blocks"].append(get_header(workflow_status))

    slack_payload["blocks"].append(get_report_section())

    failed_jobs_section = get_failed_jobs_section(failed_jobs)
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

    xml_files = []
    if args.junit_xml_dir:
        xml_files = get_xml_files(args.junit_xml_dir)
    payload = generate_payload(failed_jobs, xml_files)
    write_payload_to_file(payload)
    
if __name__ == "__main__":
    main()