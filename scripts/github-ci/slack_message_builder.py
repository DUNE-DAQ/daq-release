from __future__ import annotations
import sys
import json
import argparse
import re
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional
from abc import ABC, abstractmethod

#from integtest_xml_parser import JUnitXMLParser

STATUS_EMOJIS = {
    "success": ":white_check_mark:",
    "failure": ":rotating_light:",
    "cancelled": ":no_entry:",
    "unknown": ":question:",
}

def choose_strategy(workflow: str):
    if workflow == "Integration test workflow":
        return IntegrationTestMessageStrategy()
    else:
        return DefaultMessageStrategy()

def get_release_type(release_name):
    if "NFD_" in release_name:
        return "nightly"
    elif "rc" in release_name:
        return "candidate"
    else:
        return "stable"

def get_workflow_event(event, actor):
    if event == "schedule":
        return "This was a scheduled workflow."
    elif event == "workflow_dispatch":
        return f"This workflow was manually triggered by user `{actor}`."
    else:
        return "This workflow triggered under mysterious circumstances. Someone should investigate!"

class MessageStrategy(ABC):
    @abstractmethod
    def build(self, summary, release="unknown"):
        pass

class BaseMessageStrategy(MessageStrategy):
    def build(self, summary: dict, release: str, output_path: str = "slack_payload.json"):
        emoji = STATUS_EMOJIS.get(summary['conclusion'], ":question:")

        builder = SlackMessageBuilder(summary)

        builder.add_block(self.build_header(emoji, summary))
        builder.add_block(self.build_release_section(release))
        builder.add_block(self.build_report_section(summary))

        # Classes which inherit from BaseMessageStrategy can override this for additional information
        extra_blocks = self.build_extra_blocks(summary)
        for block in extra_blocks:
            builder.add_block(block)

        builder.add_block(self.build_footer(summary))

        builder.write(output_path)

    def build_header(self, emoji, summary):
        return HeaderBlock(f"{emoji} {summary['conclusion'].capitalize()}: {summary['workflow']} {emoji}")

    def build_release_section(self, release):
        release_type = get_release_type(release)
        return SectionBlock(f"This workflow was run using the {release_type} release `{release}`")

    def build_report_section(self, summary):
        return SectionBlock(f"Full report: <{summary['html_url']}|link>.")

    def build_footer(self, summary):
        return FooterBlock(get_workflow_event(summary['event'], summary['actor']))

    def build_extra_blocks(self, summary):
        return []


class DefaultMessageStrategy(BaseMessageStrategy):
    pass

class IntegrationTestMessageStrategy(BaseMessageStrategy):
    def build_extra_blocks(self, summary):
        pytest_log_dir = "test"
        return [
            SectionBlock(
                "The pytest logs for these tests will be stored for 7 days at:\n"
                "`daq.fnal.gov:{pytest_log_dir}`\n\n"
                "You can also download logs from the *Full report* link above."
            )
        ]


class Block(ABC):
    @abstractmethod
    def to_dict(self) -> dict:
        pass

class HeaderBlock(Block):
    def __init__(self, text: str):
        self.text = text

    def to_dict(self) -> dict:
        return {
            "type": "header",
            "text": {
                "type": "plain_text",
                "text": self.text,
                "emoji": True
            }
        }

class SectionBlock(Block):
    def __init__(self, text: str):
        self.text = text
    
    def to_dict(self) -> dict:
        return {
            "type": "section",
            "text": {
                "type": "mrkdwn",
                "text": self.text
            }
        }

class FooterBlock(Block):
    def __init__(self, text: str):
        self.text = text

    def to_dict(self) -> dict:
        return {
            "type": "context",
            "elements": [
                {
                    "type": "mrkdwn",
                    "text": self.text
                }
            ]
        }


class SlackMessageBuilder:
    STATUS_EMOJIS = {
        "success": ":white_check_mark:",
        "failure": ":rotating_light:",
        "cancelled": ":no_entry:",
    }

    def __init__(self, workflow_summary, release_name=None, junit_xml_dir=None, pytest_log_dir=None):
        self.blocks: list[Block] = []
        self.workflow_summary: dict  = workflow_summary
        self.release_name: str = release_name
        self.pytest_log_dir: Path = Path(pytest_log_dir) if pytest_log_dir else None
        #self.junit_xml_dir = junit_xml_dir
        #self.xml_parser = JUnitXMLParser(self.junit_xml_dir) if self.junit_xml_dir else None
        #self.xml_files = self.xml_parser.get_xml_files() if self.xml_parser else []

    def add_block(self, block: Block):
        self.blocks.append(block)

    def to_dict(self) -> dict:
        return {
            "blocks": [block.to_dict() for block in self.blocks]
        }

    def write(self, path: str):
        path = Path(path)
        with path.open("w") as f:
            json.dump(self.to_dict(), f)

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
        results = self.xml_parser.parse_xml_file(xml_file)
        failed_line_pattern = r"\n>\s+(.*?)\n"

        for result in results:
            if not result.get('failure_message'):
                continue
            failure_summary += f"\n\t\t  *{result['test_name']}* failed"
            failed_line_match = re.findall(failed_line_pattern, result['failure_message'])
            if failed_line_match:
                failure_summary += f" while checking \n\t\t`{failed_line_match[0]}`\n"
        return failure_summary


def main():
    parser = argparse.ArgumentParser(description="Parse a workflow summary and generate a Slack message payload.")
    parser.add_argument("--workflow-summary", required=True,
                        help="JSON file containing a workflow summary. Output from get_workflow_summary.sh")
    parser.add_argument("--release-name", required=True, default=None,
                        help="Optional name of the release used in the caller workflow.")
    parser.add_argument("--junit-xml-dir", type=str, default=None,
                        help="Optional directory containing JUnit XML files.")
    parser.add_argument("--pytest-log-dir", type=str, default=None,
                        help="Optional directory for full pytest logs.")
    parser.add_argument("--output-path", type=str, default="slack_payload.json",
                        help="Optional path/name for output json file.")
    args = parser.parse_args()

    if len(sys.argv) == 1:
        parser.print_usage()

    try:
        with open(args.workflow_summary, "r") as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        print(f"Error decoding GH API output: {e}")
        return

    message_strategy = choose_strategy(data['workflow'])
    message_strategy.build(data, args.release_name)

    if Path('slack_payload.json').is_file():
        print("Slack payload saved to slack_payload.json")
    else:
        raise FileNotFoundError("There was a problem writing slack_payload.json")

if __name__ == "__main__":
    main()
