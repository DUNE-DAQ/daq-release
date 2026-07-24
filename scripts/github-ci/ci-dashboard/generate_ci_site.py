#import re
import argparse
import json
from pathlib import Path
from datetime import datetime, timezone

from data.integration_tests import load_integration_test_data
from data.unit_tests import UnitTestReport
from data.clang_format_report import ClangFormatReport
from renderer.renderer import Renderer
from pages.page import IndexPage, RepoPage, UnitTestPage, IntegrationTestPage

def generate_site(json_input_path, artifacts_dir, core_summary='', extended_summary=''):
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

    last_updated = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")

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


    unit_test_report = UnitTestReport()
    clang_format_report = ClangFormatReport()
    repo_pages = []
    for repo_dir in sorted(Path(artifacts_dir).iterdir()):
        if not repo_dir.is_dir():
            continue
        repo_pages.append(repo_dir.name)
        unit_test_log_path = Path(repo_dir / "unit_test_summary.log")
        if unit_test_log_path.is_file():
            unit_test_report.parse(unit_test_log_path)
        clang_format_table_path = Path(repo_dir / "clang_format_summary_table.md")
        if clang_format_table_path.is_file():
            clang_format_report.parse(clang_format_table_path)

    unit_test_data = unit_test_report.to_dict()
    clang_format_data = clang_format_report.to_dict()
    integration_test_data = load_integration_test_data(core_summary, extended_summary)

    index_context = {
        "repos": repos,
        "last_updated": last_updated,
        "total_issues": total_issues,
        "total_prs": total_prs,
        "passing_percentage": passing_percentage,
        "workflow_badges": workflow_badges,
        "unit_test_summary": unit_test_data,
        "clang_format_summary": clang_format_data,
        "integration_test_summary": integration_test_data['totals'],
    }

    pages = [
        IndexPage(index_context),
        UnitTestPage(unit_test_data),
        IntegrationTestPage(integration_test_data),
    ]
    for repo_name in repo_pages:
        pages.append(RepoPage(repo_name, {
            "repo": repo_name,
            "unit_test_summary": unit_test_data["repo_results"].get(repo_name),
            "clang_format_summary": clang_format_data["repo_results"].get(repo_name),
        }))

    renderer = Renderer(repo_pages)
    for page in pages:
        page.render(renderer)

    print('Done')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate CI HTML site from a JSON summary file.")
    parser.add_argument("--json_input", required=True, help="Path to the JSON file containing CI summary data. See collect-ci-metrics.sh.")
    parser.add_argument("--artifacts_dir", required=True, help="Path to single-repo artifacts directory output by collect-ci-metrics.sh.")
    parser.add_argument("--core_pytest_summary", required=False, help="Path to the json summary output by core integration test workflow.")
    parser.add_argument("--extended_pytest_summary", required=False, help="Path to the json summary output by extended integration test workflow.")
    args = parser.parse_args()

    if not Path(args.artifacts_dir).is_dir():
        raise FileNotFoundError

    generate_site(args.json_input, args.artifacts_dir, args.core_pytest_summary, args.extended_pytest_summary)

