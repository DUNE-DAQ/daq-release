import re
from dataclasses import dataclass, field

@dataclass
class LintingReport:

    # repo -> file -> {"errors": [...], "warnings": [...]}
    repo_results: dict[str, dict[str, dict[str, list[tuple[str, str]]]]] = field(default_factory=dict)


    CPPLINT_PATTERN   = re.compile(r"^sourcecode/\S+?:(\d+):\s+(.+?)\s+\[\S+\] \[\d\]$")
    CLANGTIDY_PATTERN = re.compile(r"^\S+?:(\d+):\d+: warning: (.+?) \[\S+\]$")
    FILE_PATTERN      = re.compile(r"^Applying \S+ to sourcecode/(\w+)/(.+)$")
    ERRORS_PATTERN    = re.compile(r"Total errors found: (\d+)")

    def _flush_warnings(self, repo_name, current_file, pending_warnings):
        if repo_name and current_file and pending_warnings:
            if current_file not in self.repo_results.get(repo_name, {}):
                self.repo_results[repo_name][current_file] = {"errors": [], "warnings": []}
            self.repo_results[repo_name][current_file]["warnings"] = pending_warnings

    def parse(self, log_path):
        """Parse linting report and store results in a dictionary.

        Parsing the linting report is a little complicated due to its format,
        but basically the idea is to collect errors and warnings for each file
        if errors/warnings are present.

        repo_results = {
            "repo_name": {
                "file_1": {"errors": [...], "warnings": [...],
                "file_2": {"errors": [...], "warnings": [...],
                ...
            }
        }
        """
        current_file     = None
        repo_name        = None
        pending_errors   = []
        pending_warnings = []

        with open(log_path, "r") as f:
            for line in f:
                line = line.strip()

                file_match = self.FILE_PATTERN.match(line)
                if file_match:
                    self._flush_warnings(repo_name, current_file, pending_warnings)
                    repo_name    = file_match.group(1)
                    current_file = file_match.group(2)
                    pending_errors   = []
                    pending_warnings = []
                    if repo_name not in self.repo_results:
                        self.repo_results[repo_name] = {}
                    continue

                if current_file is None or repo_name is None:
                    continue

                if line.startswith("Total errors found:"):
                    count = int(line.split(":")[-1].strip())
                    if count > 0:
                        if current_file not in self.repo_results[repo_name]:
                            self.repo_results[repo_name][current_file] = {"errors": [], "warnings": []}
                        self.repo_results[repo_name][current_file]["errors"] = pending_errors
                    continue

                cpplint_match = self.CPPLINT_PATTERN.match(line)
                if cpplint_match:
                    pending_errors.append((cpplint_match.group(1), cpplint_match.group(2)))
                    continue

                clangtidy_match = self.CLANGTIDY_PATTERN.match(line)
                if clangtidy_match:
                    pending_warnings.append((clangtidy_match.group(1), clangtidy_match.group(2)))

        self._flush_warnings(repo_name, current_file, pending_warnings)

    def to_dict(self):
        return {"repo_results": self.repo_results}

if __name__ == "__main__":
    pass
