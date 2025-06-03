import re
import argparse
import json
from jinja2 import Environment, FileSystemLoader
from pathlib import Path
from datetime import datetime

def strip_ansi(line):
    """Strip color coding from unit test summary log."""
    return re.sub(r"\x1B\[[0-9;]*[a-zA-Z]", "", line)

def parse_unit_test_summary(log_path):
    """Summarize unit test summary as a dictionary."""
    content_by_package = {}
    current_package = None

    with open(log_path, "r") as f:
        for line in f:
            clean_line = strip_ansi(line.strip())

            if "No unit tests" in clean_line:
                this_package = clean_line.split()[8]
                test_name = None
            else:
                parts = clean_line.split("/")
                this_package = parts[1]
                test_name = parts[3].split(".")[0]

            if this_package not in content_by_package:
                content_by_package[this_package] = []

            if "SUCCESS" in clean_line:
                content_by_package[this_package].append((test_name, "Passed"))
            elif "FAILURE" in clean_line:
                content_by_package[this_package].append((test_name, "Failed"))
            elif test_name is None:
                content_by_package[this_package].append((None, "NoTests"))

    print('UNIT TEST CONTENT:', content_by_package)
    return content_by_package

def parse_integration_test_summary(pytest_summary):
    with open(pytest_summary, "r") as f:
        lines = [line.strip() for line in f if line.strip()]
        header_lines = lines[:5]
        results_lines = lines[5:]

    numbers = [int(re.search(r'\d+', line).group()) for line in header_lines]
    total, passed, failed, skipped, errors = numbers
    print('numbers', numbers)
    summary = {
        "totals": {
            "total": total,
            "passed": passed,
            "failed": failed,
            "skipped": skipped,
            "errors": errors,
        },
        "results": {}
    }
    print('numbers again', summary['totals'])
    
    # Parse test results
    i = 0
    while i < len(results_lines):
        line = results_lines[i]
        if line.startswith("#"):
            # Each section represents a new package
            section_title = re.sub(r"^#+ ", "", line)
            summary["results"][section_title] = []
            i += 3  # Skip table header and separator

            # Read test rows until next heading or end of list
            while i < len(results_lines) and not results_lines[i].startswith("#"):
                row = results_lines[i]
                if "|" in row:
                    columns = [c.strip() for c in row.strip("|").split("|")]
                    if len(columns) == 2:
                        test_name, status_col = columns
                        status = "passed" if "passed" in status_col else "failed" if "failed" in status_col else "skipped" if "skipped" in status_col else "error"
                        summary["results"][section_title].append({
                            "name": test_name,
                            "status": status
                        })
                i += 1
        else:
            i += 1

    print('pytest summary:', summary)
    return summary

def generate_site(json_input_path, unit_test_summary='', pytest_summary=''):
    """Render html files from templates to generate the site."""
    with open(json_input_path, 'r') as f:
        repos = json.load(f)

    env = Environment(loader=FileSystemLoader("templates"))
    index_template = env.get_template("index_template.html")

    total_issues = sum(repo["open_issues"] for repo in repos)
    total_prs = sum(repo["open_prs"] for repo in repos)

    total_repos = len(repos)
    passing_repos = sum(
        1 for repo in repos
        if repo.get("build_develop", {}).get("conclusion") == "success"
    )

    passing_percentage = round((passing_repos / total_repos) * 100, 1) if total_repos else 0

    last_updated=datetime.utcnow().strftime("%Y-%m-%d %H:%M UTC")
    workflow_badges = [
    {
        "image": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/build-nightly-release-alma9.yml/badge.svg",
        "link": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/build-nightly-release-alma9.yml",
        "alt": "AL9 Spack Nightly Workflow"},
    {
        "image": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/integration_tests.yml/badge.svg",
        "link": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/integration_tests.yml",
        "alt": "Nightly v5 Integration Test Workflow"},
    {
        "image": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-dbt-tests.yml/badge.svg",
        "link": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-dbt-tests.yml",
        "alt": "Nightly daq-buildtools Workflow"},
    {
        "image": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-code-check.yml/badge.svg",
        "link": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/nightly-code-check.yml",
        "alt": "Nightly unit tests and clang format check"},
    {
        "image": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/weekly-linting.yml/badge.svg",
        "link": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/weekly-linting.yml",
        "alt": "Weekly linting"},
    ]

    # Parse and summarize unit test information
    unit_test_totals = {
        'Passed': 0,
        'Failed': 0,
        'NoTests': [],
        'Other': 0,
        'Total': 0,
    }

    repos_without_unit_tests = []
    unit_test_data = parse_unit_test_summary(unit_test_summary)

    for repo_name, tests in unit_test_data.items():
        has_tests = False
        for _, status in tests:
            if status in {'Passed', 'Failed'}:
                unit_test_totals[status] += 1
                unit_test_totals['Total'] += 1
                has_tests = True
            elif status == 'NoTests':
                unit_test_totals['NoTests'].append(repo_name)
            else:
                unit_test_totals['Other'] += 1

    # Parse integration tests
    integration_test_summary = parse_integration_test_summary(pytest_summary)
    integration_test_totals = integration_test_summary['totals']
    integration_test_results = integration_test_summary['results']
    
    # Content of the index page
    context = {
        "repos": repos,
        "last_updated": last_updated,
        "total_issues": total_issues,
        "total_prs": total_prs,
        "passing_percentage": passing_percentage,
        "links": {
            "doxygen": "https://dune-daq.github.io/docs/",
        },
        "workflow_badges": workflow_badges,
        "unit_test_totals": unit_test_totals,
        "integration_test_totals": integration_test_summary['totals'],
    }


    index_template = env.get_template("index_template.html")
    index_html = index_template.render(context)
    index_path = Path("site/index.html")
    index_path.parent.mkdir(parents=True, exist_ok=True)
    index_path.write_text(index_html)

    unit_test_template = env.get_template("unit_test_template.html")
    unit_test_html = unit_test_template.render(unit_test_data=unit_test_data, no_tests=unit_test_totals['NoTests'], links=context["links"])
    unit_test_path = Path("site/unit_test_summary.html")
    unit_test_path.parent.mkdir(parents=True, exist_ok=True)
    unit_test_path.write_text(unit_test_html)

    integtest_template = env.get_template("integration_test_template.html")
    integtest_html = integtest_template.render(integration_test_results=integration_test_summary['results'], links=context["links"])
    integtest_path = Path("site/integtest_summary.html")
    integtest_path.parent.mkdir(parents=True, exist_ok=True)
    integtest_path.write_text(integtest_html)

    print('Done')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate CI HTML site from a JSON summary file.")
    parser.add_argument("--json_input", required=True, help="Path to the JSON file containing CI summary data. See collect-ci-metrics.sh.")
    parser.add_argument("--unit_test_summary", required=False, help="Path to the unit test summary output by dbt-build --unittest.")
    parser.add_argument("--pytest_summary", required=False, help="Path to the pytest markdown summary output by integration test workflow.")
    args = parser.parse_args()

    generate_site(args.json_input, args.unit_test_summary, args.pytest_summary)