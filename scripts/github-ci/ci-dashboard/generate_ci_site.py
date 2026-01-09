import re
import argparse
import json
from jinja2 import Environment, FileSystemLoader
from pathlib import Path
from datetime import datetime

from data.integration_tests import load_integration_test_data
from data.unit_tests import parse_unit_test_summary
from renderer.renderer import Renderer
from pages.page import Page, IndexPage, UnitTestPage, IntegrationTestPage

def generate_site(json_input_path, unit_test_summary='', pytest_summary=''):
    """Render html files from templates to generate the site."""
    with open(json_input_path, 'r') as f:
        repos = json.load(f)

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
        "image": "https://github.com/DUNE-DAQ/docs/actions/workflows/build-and-publish-doxygen.yml/badge.svg",
        "link": "https://github.com/DUNE-DAQ/docs/actions/workflows/build-and-publish-doxygen.yml",
        "alt": "Doxygen"},
    {
        "image": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/weekly-linting.yml/badge.svg",
        "link": "https://github.com/DUNE-DAQ/daq-release/actions/workflows/weekly-linting.yml",
        "alt": "Weekly linting"},
    ]

    unit_test_data = parse_unit_test_summary(unit_test_summary)
    integration_test_data = load_integration_test_data(pytest_summary)
    
    index_context = {
        "repos": repos,
        "last_updated": last_updated,
        "total_issues": total_issues,
        "total_prs": total_prs,
        "passing_percentage": passing_percentage,
        "workflow_badges": workflow_badges,
        "unit_test_summary": unit_test_data,
        "integration_test_summary": integration_test_data["totals"],
    }

    pages = [
        IndexPage(index_context),
        UnitTestPage(unit_test_data.to_dict()),
        IntegrationTestPage(integration_test_data),
    ]

    renderer = Renderer()
    for page in pages:
        page.render(renderer)

    print('Done')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate CI HTML site from a JSON summary file.")
    parser.add_argument("--json_input", required=True, help="Path to the JSON file containing CI summary data. See collect-ci-metrics.sh.")
    parser.add_argument("--unit_test_summary", required=False, help="Path to the unit test summary output by dbt-build --unittest.")
    parser.add_argument("--pytest_summary", required=False, help="Path to the json summary output by integration test workflow.")
    args = parser.parse_args()

    generate_site(args.json_input, args.unit_test_summary, args.pytest_summary)