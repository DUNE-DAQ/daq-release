from jinja2 import Environment, FileSystemLoader
from pathlib import Path
from datetime import datetime

def commit_age(time_since_last_commit):
    """Categorize the age of the latest commit."""
    week = 7 * 24 * 60 * 60
    month = 30 * 24 * 60 * 60

    if time_since_last_commit < week:
        return ("This week", "status-fresh")
    elif time_since_last_commit < month:
        return ("This month", "status-aging")
    else:
        return ("More than a month ago", "status-stale")

class Renderer:
    def __init__(self, repo_names=None):
        self.templates_path = Path(__file__).parent.parent / "templates"
        self.output_path = Path("site")
        self.links = {
            "Home": "index.html",
            "Doxygen": "https://dune-daq.github.io/docs/",
            "Unit Test Summary": "unit_test_summary.html",
            "Integration Test Summary": "integtest_summary.html",
            "Code Coverage Report": "code_coverage/index.html",
        }
        self.env = Environment(loader=FileSystemLoader(self.templates_path))
        self.env.filters["commit_age"] = commit_age
        self.env.globals["links"] = self.links
        self.env.globals["repo_links"] = {
            repo: f"repos/{repo}.html"
            for repo in (repo_names or [])
        }

    def render_page(self, template_name, context, output_file):
        template = self.env.get_template(template_name)
        page_html = template.render(context)
        file_path = self.output_path / output_file
        file_path.parent.mkdir(parents=True, exist_ok=True)
        file_path.write_text(page_html)

