#!/usr/bin/env python3

from __future__ import annotations
import sys
import yaml
import argparse
import tempfile
import re
import tempfile
import pathlib

from pathlib import Path
from dataclasses import dataclass, field
from tempfile import TemporaryDirectory

from mappings import cmake_to_spack
sys.path.append(str(Path(__file__).resolve().parent.parent))
from run_command import run_command

class MyDumper(yaml.Dumper):

    def increase_indent(self, flow=False, indentless=False):
        return super(MyDumper, self).increase_indent(flow, False)

def load_release_data(file: str) -> dict:
    release_yaml = Path(file)
    umbrella = release_yaml.parent.stem.split('-')[0]

    with release_yaml.open() as f:
        release_data = yaml.safe_load(f)

    release_data['umbrella'] = umbrella

    return release_data

@dataclass(frozen=True)
class ReleaseContext:
    release_name: str
    core_release: str
    full_umbrella: str
    update_hashes: bool
    overwrite_branch: str
    overwrite_daq_cmake: str


@dataclass
class DAQPackage:
    name: str
    version: str
    subset: str = None
    commit: str = None
    source: str = None
    variant: str = None
    context: ReleaseContext = None
    contains_oks_file: bool = field(default=False)
    cmake_dependencies: list[str] = field(default_factory=list)
    ref: str = field(init=False)

    def __post_init__(self):
        self.ref = self._resolve_ref()
        self._set_ref()

    # Keep a reference to the original data for when mutations are necessary
    _raw: dict | None = field(default=None, repr=False)

    @property
    def is_pymodule(self) -> bool:
        return bool(self.source)

    @property
    def is_dunedaq_pymodule(self) -> bool:
        return bool(self.source == "github_DUNE-DAQ")

    @property
    def is_external(self) -> bool:
        return self.subset in ["devtools", "externals", "systems"]

    @property
    def version_is_tag(self) -> bool:
        return bool(re.search('\d+.\d+', self.version))

    @property
    def repo_url(self) -> str:
        if self.is_pymodule and not self.is_dunedaq_pymodule:
            return None
        return f"https://github.com/DUNE-DAQ/{self.name}"

    @property
    def possible_subset_qualifier(self) -> str:
        if self.subset == "externals":
            return f', when="subset={self.context.full_umbrella}"'
        return ""

    @property
    def spec(self) -> str:
        spec_version = (
            self.version 
            if (self.version_is_tag and self.name != "daq-cmake")
            else self.context.release_name
        )

        parts = [f"{self.name}@{spec_version}"]

        if self.variant:
            parts.append(self.variant)

        base_spec = " ".join(parts)

        if self.is_external:
            return f'"{base_spec}"{self.possible_subset_qualifier}'

        return (
            f'"{base_spec} build_type={{build_type}}", '
            f'when=f"build_type={{build_type}}"'
        )

    @property
    def pyvenv_requirements_line(self) -> str:
        if not self.is_pymodule:
            return None

        if not self.is_dunedaq_pymodule:
            return f"{package.name}=={package.version}"

        user = self.source.replace("github_", "")
        ref = self.get_commit_hash() if self.ref == "develop" else self.ref
        return f"git+https://github.com/{user}/{self.name}@{ref}#egg={self.name}"

    # Update commit in both the DAQPackage object and the upstream DAQRelease.release_dict
    def set_commit(self, commit: str):
        self.commit = commit
        self._raw["commit"] = commit

    def _resolve_ref(self, fall_back_ref: str = "develop") -> str:
        if self.version_is_tag:
            # Python packages don't have a "v" in the release manifest
            # since they need to be pip-installed, but we tag them
            # with a "v" on GitHub
            if self.is_dunedaq_pymodule and self.name != "py-moo":
                return f"v{self.version}"
            if self.name == "daq-cmake" and self.context.overwrite_daq_cmake:
                return self.context.overwrite_daq_cmake
            return self.version

        overwrite = self.context.overwrite_branch
        if not overwrite:
            return fall_back_ref
        
        branch_exists = run_command(
            f"git ls-remote --exit-code --heads {self.repo_url} {overwrite}",
            continue_on_error=True
        )

        return overwrite if branch_exists["exit_code"] == 0 else fall_back_ref

    def _set_ref(self):
        self.version = self.ref
        # Python versions should not have a "v" in the final release manifest
        if not self.is_dunedaq_pymodule:
            self._raw["version"] = self.ref

    def update_commit_hash(self, repo_path_name):
        result = run_command(f"git rev-parse --short HEAD", cwd=repo_path_name)
        self.set_commit(result['stdout'])

    def get_contains_oks_file(self, repo_path_name):
        assert Path(repo_path_name).is_dir(), f"The {get_contains_oks_file.__name__} function is unable to find expected path {repo_path_name}"

        repo_path = pathlib.PosixPath(repo_path_name)

        for glob_extension in ["*.schema.xml", "*.data.xml"]:
            if len(list(repo_path.rglob(glob_extension))) > 0:
                self.contains_oks_file = True

    def get_cmake_dependencies(self, repo_path_name: str):
        if self.is_pymodule:
            return
        cmakelists = Path(f"{repo_path_name}/CMakeLists.txt")
        if not cmakelists.is_file():
            raise FileNotFoundError(cmakelists)

        content = cmakelists.read_text()

        # Parse package names from find_package calls. Everything up to the first
        # white space character will be taken as the package name (i.e., no "REQUIRED"
        # or "COMPONENTS"
        find_package_pattern = re.compile(r'\s*[^# ]\s*find_package\(\s*([^)\s]+)')
        cmake_package_list = find_package_pattern.findall(content)

        # Special cases where the dependency has no explicit find_package call
        if re.search(r'\s*[^# ]\s*daq_codegen\(', content):
            cmake_package_list.append('py-moo')
        if re.search(r'\s*[^# ]\s*daq_add_python_bindings\(', content):
            cmake_package_list.append('pybind11')
        if re.search(r'\s*[^# ]\s*pkg_check_modules\(numa', content):
            cmake_package_list.append('numactl')

        # Handle cases where the dependency name in CMakeLists.txt
        # doesn't match what Spack needs in its depends_on call
        self.cmake_dependencies = [
            cmake_to_spack.get(dep.lower(), dep.lower())
            for dep in cmake_package_list
        ]

    def get_git_info(self, fall_back_tag="develop"):
        with tempfile.TemporaryDirectory() as tmpdir:
            run_command(f"git clone --depth 1 --branch {self.ref} {self.repo_url} {tmpdir}")

            if self.context.update_hashes:
                self.update_commit_hash(tmpdir)
            self.get_contains_oks_file(tmpdir)
            self.get_cmake_dependencies(tmpdir)

@dataclass
class UmbrellaPackage:
    name: str
    is_dunedaq: bool
    template_path: str
    packages: list[DAQPackage]
    context: ReleaseContext

    def render(self):
        template_file = Path(self.template_path) / "packages" / self.name / "package.py"
        content = template_file.read_text()
        
        content = content.replace("XRELEASEX", self.context.release_name)
        content = content.replace("XTARGETX", self.context.full_umbrella)

        indent = "    "
        deps = []
        if self.is_dunedaq:
            deps.append(f'{indent}for build_type in ["Debug", "RelWithDebInfo", "Release"]:')
            indent += "    "
            if self.name != "coredaq":
                for var in ["+dev", "~dev"]:
                    deps.append(
                        f'{indent}depends_on(f"coredaq@{self.context.core_release} '
                        f'subset={self.context.full_umbrella} build_type={{build_type}} {var}", '
                        f'when=f"build_type={{build_type}} {var}")'
                    )

        for package in self.packages:
            if package.name == "dbe": continue
            deps.append(f'{indent}depends_on(f{package.spec})')

        return content + "\n".join(deps) + "\n"


class DAQRelease:
    def __init__(
        self,
        release_dict: dict,
        output_path: str,
        template_path: str = None,
        release_name: str = None,
        update_hashes: bool = False,
        overwrite_branch: str = None,
        overwrite_daq_cmake: bool = False,
        core_release: str = "",
    ):
        self.release_dict = release_dict
        self.output_path = output_path
        self.template_path = template_path
        self.release_name = release_name
        self.update_hashes = update_hashes
        self.overwrite_branch = overwrite_branch
        self.overwrite_daq_cmake = overwrite_daq_cmake
        self.core_release = core_release
        self.context = self.get_context()

        self.dunedaq_packages = self._load_dunedaq_packages()

        if self.release_name is not None:
            self.release_dict["release"] = self.release_name

        if self.core_release:
            self.release_dict["core_release"] = self.core_release

    @property
    def release_type(self):
        return self.release_dict['type']

    @property
    def full_umbrella(self):
        return self.release_dict['umbrella']

    @property
    def repo_dir(self):
        return Path(self.output_path) / "spack-repo"

    @property
    def package_dir(self):
        return Path(self.repo_dir) / "packages"

    @property
    def template_dir(self):
        return Path(self.template_path) / "packages"

    @property
    def is_nightly_release(self):
        return "daq" not in self.release_dict["release"]

    def get_context(self):
        return ReleaseContext(
            release_name=self.release_name,
            core_release=self.core_release,
            full_umbrella=self.full_umbrella,
            update_hashes=self.update_hashes,
            overwrite_branch=self.overwrite_branch,
            overwrite_daq_cmake=self.overwrite_daq_cmake,
        )

    def _load_packages(self, subset, source=None):
        return [
            DAQPackage(
                _raw = entry,
                name=entry.get("name"),
                version=str(entry.get("version")),
                commit=entry.get("commit"),
                source=entry.get("source"),
                variant=entry.get("variant"),
                subset=subset,
                context=self.context
            )
            for entry in self.release_dict.get(subset, [])
            if source is None or entry.get("source") == source
        ]

    def _load_dunedaq_packages(self):
        return self._load_packages(self.release_type) + self._load_packages("pymodules", source="github_DUNE-DAQ")

    def update_package_metadata(self):
        for package in self.dunedaq_packages:
            package.get_git_info()

    def write_release_yaml(self):
        output_file = Path(f"{self.repo_dir}/{self.release_dict['release']}.yaml")
        with output_file.open("w") as outfile:
            outfile.write('---\n')
            yaml.dump(self.release_dict, outfile, Dumper=MyDumper, default_flow_style=False, sort_keys=False)

    def generate_repo_file(self):
        with Path(f'{self.repo_dir}/repo.yaml').open('w') as f:
            f.write(f"repo:\n  namespace: '{self.release_dict['release']}'\n")

    def _load_template(self, package_name):
        template_path = Path(self.template_dir) / package_name / 'package.py'
        if not template_path.is_file():
            raise FileNotFoundError(f"Template file {template_path} is not found!")
        template_content = template_path.read_text()
        return template_content

    def _write_package_file(self, package_name, content):
        package_file = Path(self.package_dir) / package_name / "package.py"
        package_file.parent.mkdir(parents=True)
        package_file.write_text(content)
        print(f"Info: package.py has been written at {package_file}.")

    def generate_daq_packages(self):
        for package in self.dunedaq_packages:
            print(f'Package {package.name} spec: {package.spec}')
            if package.is_pymodule: continue
            content = self._load_template(package.name)

            content = content.replace("XHASHX", package.commit)

            version_replace = (
                self.release_dict["release"]
                if self.is_nightly_release
                else package.version
            )
            content = content.replace("XVERSIONX", version_replace)

            depends_on_list = [f'depends_on("{dep}")' for dep in package.cmake_dependencies]
            content = content.replace("XDEPENDSX", "\n    ".join(depends_on_list))

            dbpath_replace = (
                'env.prepend_path("DUNEDAQ_DB_PATH", self.prefix + "/share")'
                if package.contains_oks_file
                else ""
            )
            content = content.replace("XDBPATHX", dbpath_replace)

            self._write_package_file(package.name, content)

    def _generate_umbrella(self, name: str, is_dunedaq: bool):
        umbrella = UmbrellaPackage(
            name=name,
            template_path=self.template_path,
            packages=self._load_packages(name),
            context=self.context,
            is_dunedaq=is_dunedaq
        )
        content = umbrella.render()
        self._write_package_file(name, content)

    def generate_external_umbrella_package(self):
        for subset in ["devtools", "externals", "systems"]:
            self._generate_umbrella(subset, is_dunedaq=False)

    def generate_daq_umbrella_package(self):
        self._generate_umbrella(self.release_type, is_dunedaq=True)

    def generate_umbrella_packages(self):
        if self.release_type == "coredaq":
            self.generate_external_umbrella_package()
        self.generate_daq_umbrella_package()

    def generate_repo(self):
        self.repo_dir.mkdir(parents=True)
        self.update_package_metadata()
        self.write_release_yaml()
        self.generate_repo_file()
        self.generate_daq_packages()
        self.generate_umbrella_packages()
        return

    def generate_pyvenv_requirements(self, output_path):
        output_file = Path(f"{output_path}/pyvenv_requirements.txt")
        pymodules = self._load_packages("pymodules")
        if not pymodules:
            raise ValueError("No pymodules found in release manifest.")

        with output_file.open('w') as f:
            for mod in pymodules:
                f.write(mod.pyvenv_requirements_line + '\n')
        return


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='make-release-repo.py',
        description="Parse DUNE DAQ release manifest files.",
        epilog="Questions and comments to jcfree@fnal.gov",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-t', '--template-path',
                        help='''path to the template directory;''')
    parser.add_argument('-b', '--overwrite-branch',
                        default="",
                        help='''feature branch to checkout;''')
    parser.add_argument('-d', '--overwrite-daq-cmake',
                         help="If --overwrite-branch is selected, also try to use daq-cmake branch (default is for this not to happen)", action='store_true')
    parser.add_argument('-i', '--input-manifest', required=True,
                        help="path to the release manifest file;")
    parser.add_argument('-r', '--release-name',
                        help="set release name;")
    parser.add_argument('-u', '--update-hash', action='store_true',
                        help="whether to update commit hash in the YAML file;")
    parser.add_argument('-c', '--check-branch', action='store_true',
                        help="check if branch exists in repo;")
    parser.add_argument('-o', '--output-path', required=True,
                        help="path to the generated spack repo;")
    parser.add_argument('--pyvenv-requirements', action='store_true',
                        help="whether to generate requirements file for pyvenv;")
    parser.add_argument('--core-release',
                        help="core release name")

    args = parser.parse_args()

    release_dict = load_release_data(args.input_manifest)
    daq_release = DAQRelease(
        release_dict, 
        args.output_path, 
        args.template_path,
        args.release_name,
        args.update_hash,
        args.overwrite_branch, 
        args.overwrite_daq_cmake,
        args.core_release,
    )

    if args.pyvenv_requirements:
        daq_release.generate_pyvenv_requirements()
    else:
        daq_release.generate_repo()
