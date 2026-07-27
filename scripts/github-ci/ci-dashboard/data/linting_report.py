import re
from dataclasses import dataclass, field
from pathlib import Path

@dataclass
class LintingReport:
    # repo_name -> file_name -> list of (line_number, message)
    repo_results: dict[str, dict[str, list[tuple[str, str]]]] = field(default_factory=dict)

    CPPLINT_PATTERN   = re.compile(r"^sourcecode/\S+?:(\d+):\s+(.+?)\s+\[\S+\] \[\d\]$")
    CLANGTIDY_PATTERN = re.compile(r"^\S+?:(\d+):\d+: warning: (.+?) \[\S+\]$")
    FILE_PATTERN      = re.compile(r"^Applying \S+ to sourcecode/(\w+)/(.+)$")

    def parse(self, log_path):
        current_file = None
        repo_name    = None

        with open(log_path, "r") as f:
            lines = f.readlines()

        for line in lines:
            line = line.strip()

            file_match = self.FILE_PATTERN.match(line)
            if file_match:
                repo_name    = file_match.group(1)
                current_file = file_match.group(2)
                if repo_name not in self.repo_results:
                    self.repo_results[repo_name] = {}
                if current_file not in self.repo_results[repo_name]:
                    self.repo_results[repo_name][current_file] = []
                continue

            if current_file is None or repo_name is None:
                continue

            for pattern in (self.CPPLINT_PATTERN, self.CLANGTIDY_PATTERN):
                match = pattern.match(line)
                if match:
                    line_number = match.group(1)
                    message     = match.group(2)
                    self.repo_results[repo_name][current_file].append((line_number, message))
                    break

    def to_dict(self):
        return {"repo_results": self.repo_results}

if __name__ == "__main__":
    pass
