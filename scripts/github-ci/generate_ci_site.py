from jinja2 import Environment, FileSystemLoader
import json
from pathlib import Path
from datetime import datetime

with open("ci_summary.json") as f:
    repos = json.load(f)

env = Environment(loader=FileSystemLoader("templates"))
template = env.get_template("index.html")

#output_html = template.render(repos=repos)
output_html = template.render(
    repos=repos_data,
    last_updated=datetime.utcnow().strftime("%Y-%m-%d %H:%M UTC"),
    total_issues=total_issues,
    total_prs=total_prs,
    passing_percentage=passing_percentage
)


output_path = Path("output/index.html")
output_path.parent.mkdir(parents=True, exist_ok=True)
output_path.write_text(output_html)

print(f"Dashboard html file written to {output_path}")
