import argparse
import json
from jinja2 import Environment, FileSystemLoader
from pathlib import Path
from datetime import datetime
from generate_unit_test_summary import parse_unit_test_summary

def generate_site(json_input_path, unit_test_summary=''):

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

    unit_test_totals = {
        'Passed': 0,
        'Failed': 0,
        #'NoTests': 0,
        'NoTests': [],
        'Other': 0,
        'Total': 0,
    }

    repos_without_unit_tests = []
    unit_test_data = parse_unit_test_summary(unit_test_summary)

    for repo_name, tests in unit_test_data.items():
        has_tests = False
        for _, status in tests:
            #if status in unit_test_totals:
            #    unit_test_totals[status] += 1
            #else:
            #    unit_test_totals['Other'] += 1

            if status in {'Passed', 'Failed'}:
                unit_test_totals[status] += 1
                unit_test_totals['Total'] += 1
                has_tests = True
            elif status == 'NoTests':
                unit_test_totals['NoTests'].append(repo_name)
            else:
                unit_test_totals['Other'] += 1
        #if not has_tests:
        #    repos_without_unit_tests.append(repo_name)
    
    print('Repos without tests:', unit_test_totals['NoTests'])

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
    }

    index_template = env.get_template("index_template.html")
    index_html = index_template.render(context)
    index_path = Path("site/index.html")
    index_path.parent.mkdir(parents=True, exist_ok=True)
    index_path.write_text(index_html)

    unit_test_template = env.get_template("unit_test_template.html")
    print('Unit test data:', unit_test_data)
    unit_test_html = unit_test_template.render(unit_test_data=unit_test_data, no_tests=unit_test_totals['NoTests'], links=context["links"])
    unit_test_path = Path("site/unit_test_summary.html")
    unit_test_path.parent.mkdir(parents=True, exist_ok=True)
    unit_test_path.write_text(unit_test_html)


    print(f"CI site html file written to {index_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate CI HTML site from a JSON summary file.")
    parser.add_argument("--json_input", required=True, help="Path to the JSON file containing CI summary data. See collect-ci-metrics.sh.")
    parser.add_argument("--unit_test_summary", required=False, help="Path to the markdown or html table summarizing unit test results.")
    args = parser.parse_args()

    generate_site(args.json_input, args.unit_test_summary)