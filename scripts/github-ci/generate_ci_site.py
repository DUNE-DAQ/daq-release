import argparse
import json
from jinja2 import Environment, FileSystemLoader
from pathlib import Path
from datetime import datetime

def generate_site(json_input_path):

    with open(json_input_path, 'r') as f:
        repos = json.load(f)

    env = Environment(loader=FileSystemLoader("templates"))
    template = env.get_template("index_template.html")

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
    }

    output_html = template.render(context)
    output_path = Path("site/index.html")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(output_html)

    print(f"Dashboard html file written to {output_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate CI HTML site from a JSON summary file.")
    parser.add_argument("--json_input", help="Path to the JSON file containing CI summary data. See collect-ci-metrics.sh.")
    args = parser.parse_args()

    generate_site(args.json_input)