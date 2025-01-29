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

def generate_payload(failed_jobs, xml_files=[]):
    """
    Top-level payload generator function that determines which payload to generate based on 
    the presence of job failures. The failure information is parsed to generate a message 
    payload in Slack's BlockKit format.  
    """
    slack_payload = None
    if not failed_jobs:
        slack_payload = create_success_payload()
    else:
        slack_payload = create_failure_payload(failed_jobs)
    return slack_payload

def create_success_payload():
    return {
        "blocks": [
            {
            "type": "header",
            "text": {
                "type": "plain_text",
                "text": ":white_check_mark: Success: ${{ github.workflow }} :white_check_mark:",
                "emoji": True
            }
            },
            {
            "type": "section",
            "text": {
                "type": "mrkdwn",
                "text": "Full report: <https://github.com/${{ github.payload.repository.full_name }}/actions/runs/${{ github.runId }}|link>."
            }
            }
        ]
    }

def create_failure_payload(failed_jobs):
    slack_failure_payload = {
        "blocks": [
            {
            "type": "header",
            "text": {
                "type": "plain_text",
                "text": ":rotating_light: Failure: ${{ github.workflow }} :rotating_light:",
                "emoji": True
            }
            },
            {
            "type": "section",
            "text": {
                "type": "mrkdwn",
                "text": "Full report: <https://github.com/${{ github.payload.repository.full_name }}/actions/runs/${{ github.runId }}|link>."
            }
            }
        ]
    }
    failed_jobs_text = "*Failed jobs and steps:*\n"
    for job in failed_jobs:
        failed_jobs_text += f"*Job name*: {job['job']}\n"
        failed_jobs_text += f"*Failed step(s) in this job:*\n"
        for step in job['steps']:
            failed_jobs_text += f"\t :x: {step['name']}*\n"

    slack_failure_payload["blocks"].append({
        "type": "section",
        "text": {
            "type": "mrkdwn",
            "text": failed_jobs_text
        }
    })

    return slack_failure_payload

def create_integration_test_failure_payload():
    from .integtest_xml_parser import parse_junit_xml
    pass

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