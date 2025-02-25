import sys, os
import json
import argparse
import sys
import re
from pathlib import Path
from integtest_xml_parser import parse_junit_xml

class SlackPayload:
    """Class to generate a Slack JSON payload for workflow notifications."""

    STATUS_EMOJIS = {
        "success": ":white_check_mark:",
        "failure": ":rotating_light:",
        "cancelled": ":no_entry:",
    }

    def __init__(self, workflow_summary, xml_files=None, pytest_log_dir=None):
        self.workflow_summary  = workflow_summary
        self.workflow_name     = workflow_summary['workflow']
        self.workflow_trigger  = workflow_summary['event']
        self.workflow_actor    = workflow_summary['actor']
        self.workflow_html_url = workflow_summary['html_url']
        self.failed_jobs       = workflow_summary['failed_jobs']
        self.xml_files = xml_files or []
        self.pytest_log_dir = pytest_log_dir
        self.workflow_status = self.get_workflow_status()
        self.blocks = []

    def get_workflow_status(self):
        """Determine the overall workflow status based on job conclusions."""
        has_cancelled = any(job['conclusion'] == 'cancelled' for job in self.failed_jobs)
        has_failure = any(job['conclusion'] == 'failure' for job in self.failed_jobs)
        if has_cancelled:
            return 'cancelled'
        if has_failure:
            return 'failure'
        return 'success'

    def add_header(self):
        """Add the header block displaying the workflow name and status."""
        workflow_name = self.workflow_summary['workflow']
        emoji = self.STATUS_EMOJIS.get(self.workflow_status, ":grey_question:")
        self.blocks.append({
            "type": "header",
            "text": {
                "type": "plain_text",
                "text": f"{emoji} {self.workflow_status.capitalize()}: {workflow_name} {emoji}",
                "emoji": True
            }
        })

    def add_report_section(self):
        """Add the link to full GitHub Actions output."""
        self.blocks.append({
            "type": "section",
            "text": {
                "type": "mrkdwn",
                #"text": "Full report: <https://github.com/${{ github.payload.repository.full_name }}/actions/runs/${{ github.runId }}|link>."
                "text": f"Full report: <{self.workflow_html_url}|link>."
            }
        })
        if self.workflow_name == "Weekly code coverage workflow":
            self.blocks.append({
                "type": "section",
                "text": {
                    "type": "mrkdwn",
                    "text": "You can view the full lcov coverage report <https://dune-daq.github.io/daq-release/|here>."
                }
            })

    def add_failed_jobs_section(self):
        """Sections that lists failed jobs and steps."""
        if not self.failed_jobs or self.workflow_status != 'failure':
            return
        
        failed_jobs_text = "*Failed jobs and steps:*\n"
        for job in self.failed_jobs:
            if job['conclusion'] != 'failure':
                continue
            failed_jobs_text += f"- *{job['job']}*\n"
            for step in job['steps']:
                failed_jobs_text += f"\t:x: *{step['name']}*\n"
                if 'integration_tests' in job['job']:
                    test_name = job['job'].split()[1].strip("()")
                    failed_jobs_text += self.get_integration_test_failure(test_name)

        self.blocks.append({
            "type": "section",
            "text": {"type": "mrkdwn", "text": failed_jobs_text}
        })

    def get_integration_test_failure(self, test_name):
        """Parse JUnit XML files output by integration tests to roughly determine failure reason."""
        xml_file = self.find_matching_file(test_name)
        if not xml_file:
            return "\n\t\t  *Unknown failure:* No matching results.xml file found.\n"

        failure_summary = ''
        results = parse_junit_xml(xml_file)
        failed_line_pattern = r"\n>\s+(.*?)\n"

        for result in results:
            if not result.get('failure_message'):
                continue
            failure_summary += f"\n\t\t  *{result['test_name']}* failed"
            failed_line_match = re.findall(failed_line_pattern, result['failure_message'])
            if failed_line_match:
                failure_summary += f" while checking \n\t\t`{failed_line_match[0]}`\n"
        return failure_summary

    def find_matching_file(self, test_name):
        """Find a JUnit XML file that matches the integration test name."""
        for file in self.xml_files:
            if test_name in str(file):
                return file
        return None

    def add_pytest_log_section(self):
        """Add pytest log directory section if available."""
        if self.pytest_log_dir:
            self.blocks.append({
                "type": "section",
                "text": {
                    "type": "mrkdwn",
                    "text": f"The pytest logs for these tests will be stored for 7 days at: `daq.fnal.gov:{self.pytest_log_dir}`.\n"
                             "You can also download logs from the 'Full report' link above."
                }
            })

    def add_footer(self):
        footer_text = ""
        if self.workflow_trigger == "schedule":
            footer_text = "This was a scheduled workflow."
        elif self.workflow_trigger == "workflow_dispatch":
            footer_text = f"This workflow was manually triggered by user `{self.workflow_actor}`"

        if footer_text:
            self.blocks.append({
                "type": "section",
                "text": {"type": "mrkdwn", "text": footer_text}
            })

    def to_dict(self):
        """Build and return the final Slack payload."""
        self.add_header()
        self.add_report_section()
        self.add_failed_jobs_section()
        self.add_pytest_log_section()
        self.add_footer()
        return {"blocks": self.blocks}

def write_payload_to_file(payload, file_name='slack_payload.json'):
    """Write the Slack payload to a file."""
    try:
        with open(file_name, "w") as file:
            json.dump(payload, file)
        print(f"Payload written to {file_name}")
    except Exception as e:
        print(f"Failed to write payload to file: {e}")

def main():
    parser = argparse.ArgumentParser(description="Parse a workflow summary and generate a Slack message payload.")
    parser.add_argument("--api-output", required=True,
                        help="JSON file containing a summary of failed jobs from GitHub API.")
    parser.add_argument("--junit-xml-dir", nargs="?", default=None,
                        help="Optional directory containing JUnit XML files.")
    parser.add_argument("--pytest-log-dir", nargs="?", default=None,
                        help="Optional directory for full pytest logs.")
    args = parser.parse_args()

    if len(sys.argv) == 1:
        parser.print_usage()

    try:
        with open(args.api_output, "r") as f:
            workflow_summary = json.load(f)
    except json.JSONDecodeError as e:
        print(f"Error decoding GH API output: {e}")
        return

    xml_files = []
    if args.junit_xml_dir and os.path.isdir(args.junit_xml_dir):
        xml_files = list(Path(args.junit_xml_dir).rglob("*_results.xml"))

    slack_payload = SlackPayload(workflow_summary, xml_files, args.pytest_log_dir)
    write_payload_to_file(slack_payload.to_dict())

if __name__ == "__main__":
    main()
