import re
from dataclasses import dataclass, field

@dataclass
class LinkCheckerReport:
    # Map repo_name to a list of files with links containing errors
    repo_results: dict[str, dict[str, list[tuple[str, str]]]] = field(default_factory=dict)

    def parse(self, log_path):
        """Parse link checker output and store as a dictionary.
        The lines we're looking look something like:

        ### Errors in include/triggeralgs/MichelElectron/README.md

        * [403] <BAD_LINK> (at 3:36) | Rejected status code: 403 Forbidden

        We want to collect these into a list of files with bad links mapped by repo name
        Also, each file should map to the link and http error code

        repo_results = {
            "hdf5libs": {
                "file_1": [(link1, error1), (link2, error2)],
                "file_2": [(link1, error1), (link2, error2)],
            }
        }

        Note that LinkCheckerReport does not use a to_dict() method like the UnitTestReport
        and ClangFormatReport classes. The extra layer of wrapping the results in another
        dictionary turns out to be cumbersome and confusing.
        """

        with open(log_path, "r") as f:
            lines = [line.strip() for line in f.readlines()]

        # repo_name comes from the last line, which looks like:
        # 'Full Github Actions output](https://github.com/DUNE-DAQ/<repo_name>/actions/runs/<run_number>?check_suite_focus=true)'
        repo_name = lines[-1].split('/')[4]

        current_file = None
        pattern = re.compile(r"\[(\d+)]\s+<(\S+)>")
        for line in lines:
            if line.startswith("### Errors"):
                if repo_name not in self.repo_results:
                    self.repo_results[repo_name] = {}
                current_file = line.replace("### Errors in ", "").strip()
                if current_file not in self.repo_results[repo_name]:
                    self.repo_results[repo_name][current_file] = []
                    continue

            match = pattern.search(line)
            if match and current_file is not None:
                error_code = match.group(1)
                url        = match.group(2)
                self.repo_results[repo_name][current_file].append((url, error_code))


if __name__ == "__main__":
    pass
