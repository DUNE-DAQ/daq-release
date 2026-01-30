from __future__ import annotations
import sys
import json
import argparse
import re
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional
from abc import ABC, abstractmethod

STATUS_EMOJIS = {
    "success": ":white_check_mark:",
    "failure": ":rotating_light:",
    "cancelled": ":no_entry:",
    "unknown": ":question:",
}

def choose_strategy(data: dict):
    workflow_name = data['workflow']
    if workflow_name == "Integration test workflow":
        print("Integtest strat")
        return IntegrationTestMessageStrategy(data)
    else:
        print("Default strat")
        return DefaultMessageStrategy(data)

def get_release_type(release_name):
    if not release_name:
        return "Unknown"
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
    def __init__(self, summary: dict):
        self.summary = summary

    @abstractmethod
    def build(self, output_path: str = "slack_payload.json"):
        pass

class BaseMessageStrategy(MessageStrategy):
    def build(self, output_path: str = "slack_payload.json"):
        emoji = STATUS_EMOJIS.get(self.summary['conclusion'], ":question:")

        builder = SlackMessageBuilder()

        builder.add_block(self.build_header(emoji))
        builder.add_block(self.build_report_section())

        if self.summary['failed_jobs']:
            builder.add_block(self.build_failed_jobs_block())

        # Classes which inherit from BaseMessageStrategy can override this for additional information
        extra_blocks = self.build_extra_blocks()
        for block in extra_blocks:
            builder.add_block(block)

        builder.add_block(self.build_footer())

        print('OUTPUT PATH:', output_path)
        builder.write(output_path)

    def build_header(self, emoji):
        return HeaderBlock(f"{emoji} {self.summary['conclusion'].capitalize()}: {self.summary['workflow']} {emoji}")

    def build_release_section(self):
        release_type = get_release_type(self.summary['release'])
        release = self.summary['release']
        return SectionBlock(f"This workflow was run using the {release_type} release `{release}`")

    def build_report_section(self):
        return SectionBlock(f"Full report: <{self.summary['html_url']}|link>.")

    def build_failed_jobs_section(self):
        pass

    def build_footer(self):
        return FooterBlock(get_workflow_event(self.summary['event'], self.summary['actor']))

    def build_extra_blocks(self):
        return []


class DefaultMessageStrategy(BaseMessageStrategy):
    pass

class IntegrationTestMessageStrategy(BaseMessageStrategy):
    def build_extra_blocks(self):
        pytest_log_dir = self.summary.get("pytest_log_dir", None)
        extra_blocks = []
        extra_blocks.append(self.build_release_section())
        extra_blocks.append(
            SectionBlock(
                f"The pytest logs for these tests will be stored for 7 days at:\n"
                f"`daq.fnal.gov:{pytest_log_dir}`\n"
                f"You can also download logs from the *Full report* link above."
            )
        )
        return extra_blocks


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

    #def __init__(self, workflow_summary=None, release_name=None, junit_xml_dir=None, pytest_log_dir=None):
    def __init__(self):
        self.blocks: list[Block] = []

    def add_block(self, block: Block):
        self.blocks.append(block)

    def to_dict(self) -> dict:
        return {
            "blocks": [block.to_dict() for block in self.blocks]
        }

    def write(self, path: str):
        print("OUTPUT PATH??", path)
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

    message_strategy = choose_strategy(data)
    message_strategy.build(args.output_path)

    if Path(args.output_path).is_file():
        print("Slack payload saved to slack_payload.json")
    else:
        raise FileNotFoundError("There was a problem writing slack_payload.json")

if __name__ == "__main__":
    main()
