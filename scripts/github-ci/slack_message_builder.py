from __future__ import annotations
import sys
import json
import argparse
import requests
import re
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional
from abc import ABC, abstractmethod

STATUS_EMOJIS = {
    "success"  : ":white_check_mark:",
    "failure"  : ":rotating_light:",
    "cancelled": ":no_entry:",
    "unknown"  : ":question:",
}

def choose_strategy(data: dict) -> MessageStrategy:
    workflow_name = data['caller']
    if "integration test workflow" in workflow_name.lower():
        return IntegrationTestMessageStrategy(data)
    elif workflow_name == "Weekly code coverage workflow":
        return CodeCoverageMessageStrategy(data)
    elif workflow_name == "Weekly linting workflow":
        return LintingMessageStrategy(data)
    elif workflow_name == "Poll CVMFS for release":
        return NewReleaseMessageStrategy(data)
    else:
        return DefaultMessageStrategy(data)

def get_release_type(release_name: str) -> str:
    if not release_name:
        return None
    if release_name.startswith("NFD_"):
        return "nightly"
    elif "FD_" in release_name and not release_name.startswith("N"):
        return "test"
    elif "rc" in release_name:
        return "candidate"
    else:
        return "stable"

def get_workflow_event(event: str, actor:str) -> str:
    if event == "schedule":
        return "This was a scheduled workflow."
    elif event == "workflow_dispatch":
        return f"This workflow was manually triggered by user `{actor}`."
    else:
        return "This workflow triggered under mysterious circumstances. Someone should investigate!"

# Using the GitHub Actions "conclusion" from gh api doesn't work because
# this script runs *during* a larger workflow. gh api therefore returns 
# "null" as the conclusion
def get_workflow_status(summary: dict) -> str:
    if summary.get('cancelled_jobs'):
        return "cancelled"
    if summary.get('failed_jobs'):
        return "failure"
    return "success"

# In the event that the instructions link moves, requests.get will
# always return 200 since GitHub wiki will redirect to "create a new page".
# Using requests.history catches the redirect.
def instructions_link_is_valid(link: str) -> bool:
    return False if requests.get(link).history else True


class MessageStrategy(ABC):
    def __init__(self, summary: dict):
        self.summary = summary

    @abstractmethod
    def build(self, output_path: str = "slack_payload.json"):
        pass

class BaseMessageStrategy(MessageStrategy):
    def build(self, output_path: str = "slack_payload.json"):
        status = get_workflow_status(self.summary)

        builder = SlackMessageBuilder()

        builder.add_block(self.build_header(status))
        builder.add_block(self.build_report_section())

        if self.summary['failed_jobs']:
            builder.add_block(self.build_failed_jobs_section())

        # Classes which inherit from BaseMessageStrategy can override this for additional information
        extra_blocks = self.build_extra_blocks()
        for block in extra_blocks:
            builder.add_block(block)

        builder.add_block(self.build_footer())

        builder.write(output_path)

    def build_header(self, status: str):
        emoji = STATUS_EMOJIS.get(status, ":question:")
        status = get_workflow_status(self.summary).capitalize()
        workflow = self.summary.get("workflow", "Unknown workflow name")
        return HeaderBlock(f"{emoji} {status}: {workflow} {emoji}")

    def build_release_section(self):
        release = self.summary.get("release", None)
        release_type = get_release_type(release)
        if not release_type:
            return SectionBlock(f":warning: Unable to get release name for this workflow. Someone should investigate!")
        return SectionBlock(f"This workflow was run using the {release_type} release `{release}`")

    def build_report_section(self):
        return SectionBlock(f"Full report: <{self.summary['html_url']}|link>.")

    def build_failed_jobs_section(self):
        failed_jobs_text = "*Failed jobs and steps:*\n"
        for failed_job in self.summary['failed_jobs']:
            failed_jobs_text += f"- {failed_job['job']}\n"
            for step in failed_job['steps']:
                failed_jobs_text += f"\t:x: *{step['name']}*\n"

        return SectionBlock(failed_jobs_text)

    def build_footer(self):
        event = self.summary.get("event", None)
        actor = self.summary.get("actor", None)
        return FooterBlock(get_workflow_event(event, actor))

    def build_extra_blocks(self):
        return []

class DefaultMessageStrategy(BaseMessageStrategy):
    pass

class NewReleaseMessageStrategy(BaseMessageStrategy):
    def __init__(self, summary: dict):
        super().__init__(summary)
        self.instructions_link = (
            "https://github.com/DUNE-DAQ/daqconf/wiki/"
            "Setting-up-a-FarDet-DAQ-software-development-area"
        )

    def build(self, output_path: str = "slack_payload.json"):
        status = get_workflow_status(self.summary)

        builder = SlackMessageBuilder()

        builder.add_block(self.build_header(status))

        if self.summary['failed_jobs']:
            builder.add_block(self.build_failed_jobs_section())
        else:
            builder.add_block(self.build_release_section())

        if not instructions_link_is_valid(self.instructions_link):
            builder.add_block(self.build_stale_link_section())

        builder.add_block(self.build_footer())

        builder.write(output_path)

    def build_header(self, status: str):
        emoji = STATUS_EMOJIS.get(status, ":question:")
        status = get_workflow_status(self.summary).capitalize()
        workflow = self.summary.get("workflow", "Unknown workflow name")
        if status == "Success":
            return HeaderBlock(f":mega: New Release on CVMFS :mega:")
        return HeaderBlock(f"{emoji} {status}: {workflow} {emoji}")

    def build_release_section(self):
        release = self.summary.get("release", None)
        release_type = get_release_type(release)
        if not release_type:
            return SectionBlock(f":warning: Unable to get release name for this workflow. Someone should investigate!")
        return SectionBlock(
            f"A DUNE-DAQ {release_type} release with tag `{release}` has appeared on CVMFS.\n"
            f"To set up a working area based on this release, follow the instructions"
            f" <{self.instructions_link}|here>."
        )

    def build_stale_link_section(self):
        return SectionBlock(
            f":warning: There is an issue with the link to the "
            f"instructions for setting up the latest development area. "
            f"Someone should investigate!"
        )

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

class CodeCoverageMessageStrategy(BaseMessageStrategy):
    def build_extra_blocks(self):
        extra_blocks = []
        extra_blocks.append(self.build_release_section())
        extra_blocks.append(
            SectionBlock(
                f"You can download the full coverage report from the \"Artifacts\" section <{self.summary['html_url']}|here>."
            )
        )
        return extra_blocks

class LintingMessageStrategy(BaseMessageStrategy):
    def build_extra_blocks(self):
        extra_blocks = []
        extra_blocks.append(self.build_release_section())
        extra_blocks.append(
            SectionBlock(
                f"You can download the full linting report from the \"Artifacts\" section <{self.summary['html_url']}|here>."
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
    def __init__(self):
        self.blocks: list[Block] = []

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
