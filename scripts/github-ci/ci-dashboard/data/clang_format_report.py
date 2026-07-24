from pathlib import Path
from dataclasses import dataclass, field

@dataclass
class ClangFormatReport:
    # Map repo_name to a list of tuples, where each tuple is ('file_name', 'status')
    repo_results: dict[str, list[tuple[str, str]]] = field(default_factory=dict)

    def parse(self, log_path):
        """Parse clang formatting summary and store as a dictionary."""
        if not Path(log_path).is_file():
            raise FileNotFoundError

        # Before parsing, the clang format markdown report looks something like
        # Clang-Format Report
        # ## hdf5libs 
        # | Test | Status| 
        # | --- | --- |
        # | sourcecode/hdf5libs/include/hdf5libs/HDF5FileLayout.hpp | :white_check_mark: Already formatted |
        # | sourcecode/hdf5libs/include/hdf5libs/HDF5FileLayoutParameters.hpp | :x: Needs formatting |
        # We want to parse this down to tuples of (<file_name>, <status>)
        with open(log_path, "r") as f:
            for line in f:
                line = line.strip()
                if "##" in line:
                    repo_name = line.lstrip("##").strip()
                if not line.startswith('|') or '---' in line or 'Test' in line:
                    continue

                if repo_name not in self.repo_results:
                    self.repo_results[repo_name] = []

                cells = [c.strip() for c in line.split('|')[1:-1]]
                cells[0] = cells[0].replace(f"sourcecode/{repo_name}/", "").strip()
                cells[1] = cells[1].split(':')[-1].strip()
                if len(cells) == 2:
                    self.repo_results[repo_name].append((cells[0], cells[1]))
        
    def to_dict(self):
        return {"repo_results": self.repo_results}


if __name__ == "__main__":
    pass
