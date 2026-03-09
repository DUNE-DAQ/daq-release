#!/usr/bin/env python3

from __future__ import annotations
import os, sys
import yaml
import argparse
import shutil
import subprocess
import tempfile
import re
import copy
import tempfile
import pathlib
import requests

from time import sleep
from pathlib import Path
from dataclasses import dataclass, field
from git import Repo
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

@dataclass
class DAQPackage:
    name: str
    version: str
    subset: str
    release_name: str
    full_umbrella: str
    commit: str = None
    source: str = None
    variant: str = None

    contains_oks_file: bool = False
    cmake_dependencies: list[str] = field(default_factory=list)

    # Keep a reference to the original data in case mutations are necessary
    _raw: dict | None = field(default=None, repr=False)

    @property
    def is_pymodule(self) -> bool:
        return bool(self.source)

    @property
    def is_dunedaq_pymodule(self) -> bool:
        return bool(self.source == "github_DUNE-DAQ")

    @property
    def is_external(self) -> bool:
        return self.kind in ["externals", "devtools", "systems"]

    @property
    def version_is_tag(self) -> bool:
        return bool(re.search('\d+.\d+.\d+', self.version))

    @property
    def repo_url(self) -> str:
        if self.is_pymodule and not self.is_dunedaq_pymodule:
            return None
        return f"https://github.com/DUNE-DAQ/{self.name}"

    @property
    def possible_subset_qualifier(self) -> str:
        if self.subset == "externals":
            return f', when="subset={self.full_umbrella}"'
        return ""

    @property
    def spec(self) -> str:
        spec_version = self.version if self.version_is_tag else self.release_name
        spec = f"{self.name}@{spec_version}"

        # External packages
        if self.variant:
            spec += f" {self.variant}"
            spec += f"{self.possible_subset_qualifier}"
            return spec

        # DUNE-DAQ packages
        spec += ' build_type={{build_type}}", when=f"build_type={{build_type}}'
        return spec

    # Update commit in both the DAQPackage object and the upstream DAQRelease.release_dict
    def set_commit(self, commit: str):
        self.commit = commit
        self._raw["commit"] = commit

    def update_commit_hash(self, repo_path_name, fall_back_tag="develop"):
        result = run_command(f"git rev-parse --short HEAD", cwd=repo_path_name)
        self.set_commit(result['stdout'])

    def get_contains_oks_file(self, repo_path_name):
        assert os.path.exists(repo_path_name), f"The {get_contains_oks_file.__name__} function is unable to find expected path {repo_path_name}"

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
            cmake_to_spack.get(dep, dep)
            for dep in cmake_package_list
        ]

    def get_git_info(self, update_package_hash):
        # Pymodules don't have a "v" in the manifest versions since 
        # they need to be pip-installed, but we tag them with the 
        # "v" on GitHub
        version = self.version
        if self.is_dunedaq_pymodule and self.version_is_tag:
            version = f"v{version}"

        with tempfile.TemporaryDirectory() as tmpdir:
            run_command(f"git clone --depth 1 --branch {version} {self.repo_url} {tmpdir}")
            if update_package_hash:
                self.update_commit_hash(tmpdir)
            self.get_contains_oks_file(tmpdir)
            self.get_cmake_dependencies(tmpdir)

@dataclass
class UmbrellaPackage:
    name: str
    full_umbrella: str
    core_release: str
    template_path: str
    packages: list[DAQPackage]

    @property
    def template_file(self):
        return Path(self.template_path / "packages" / self.name / "package.py")

    def render(self):
        content = self.template_file.read_text()
        
        content = content.replace("XRELEASEX", self.release_dict["release"])
        content = content.replace("XTARGETX", self.full_umbrella)

        for package in self.packages:
            content += package.spec

        print('UMBRELLA CONTENT:', content)
        return content

    def _dunedaq_dependencies(self, content):
        content += '\n    for build_type in ["Debug", "RelWithDebInfo", "Release"]:'
        if self.name != "coredaq":
            content += f'\n        depends_on(f"coredaq@{self.core_release} subset={self.full_umbrella} build_type={{build_type}} +dev", when=f"build_type={{build_type}} +dev")'
            content += f'\n        depends_on(f"coredaq@{self.core_release} subset={self.full_umbrella} build_type={{build_type}} ~dev", when=f"build_type={{build_type}} ~dev")'
        for package in self.packages:
            if package.name == "dbe":
                continue
            if package.name.startswith("py-"):
                iver = idep["version"]
                lines += f'\n        depends_on(f"{iname}@{iver}")'
            else:
                # Nightlies
                if "daq" not in self.rdict["release"]:
                    iver = self.rdict["release"]
                lines += f'\n        depends_on(f"{iname}@{iver} build_type={{build_type}}", when=f"build_type={{build_type}}")'
        lines += '\n'
        return content

    def _external_dependencies(self, content):
        possible_subset_qualifier=""
        if self.name == 'externals':
            possible_subset_qualifier=f', when="subset={self.full_umbrella}"'

        # now add additional deps:
        print('Generating externals:', self.name)
        print(f"{ipkg} packages: {self.packages}")
        for package in self.packages:
            if package.variant:
                content += f'\n    depends_on("{package.name}@{package.version} {package.variant}"{possible_subset_qualifier})'
            else:
                content += f'\n    depends_on("{package.name}@{package.version}"{possible_subset_qualifier})'

        return content


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

    def _load_packages(self, subset, source=None):
        print("Attempting to load", subset, "with source", source)
        print(f"{subset} exists?", self.release_dict.get(subset))
        return [
            DAQPackage(
                _raw = entry,
                name=entry.get("name"),
                version=str(entry.get("version")),
                commit=entry.get("commit"),
                source=entry.get("source"),
                variant=entry.get("variant"),
                subset=subset,
                release_name=self.release_name,
                full_umbrella=self.full_umbrella,
            )
            for entry in self.release_dict.get(subset, [])
            if source is None or entry.get("source") == source
        ]

    def _load_dunedaq_packages(self):
        return self._load_packages(self.release_type) + self._load_packages("pymodules", source="github_DUNE-DAQ")

    def update_package_metadata(self):
        #daq_packages = self._load_packages(self.release_type)
        for package in self.dunedaq_packages:
            package.get_git_info(self.update_hashes)
        return

    def write_release_yaml(self):
        output_file = Path(f"{self.repo_dir}/{self.release_dict['release']}.yaml")
        with output_file.open("w") as outfile:
            outfile.write('---\n')
            yaml.dump(self.release_dict, outfile, Dumper=MyDumper, default_flow_style=False, sort_keys=False)
        return

    def generate_depends_on_list(self, cmake_package_list):
        depends_on_list = ""
        for idep in cmake_package_list:
            # Special cases where find_package call in CMakeLists is not sufficient
            if idep in cmake_to_spack:
                idep = cmake_to_spack[idep]
            depends_on_list += f'\n    depends_on("{idep}")'
        return depends_on_list

    def generate_repo_file(self):
        with Path(f'{self.repo_dir}/repo.yaml').open('w') as f:
            f.write(f"repo:\n  namespace: '{self.release_type}'\n")
        return

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

    def generate_external_umbrella_package(self):
        for ipkg in ["devtools", "externals", "systems"]:
            content = self._load_template(ipkg)
            
            content = content.replace("XRELEASEX", self.release_dict["release"])
            content = content.replace("XTARGETX", self.full_umbrella)

            possible_subset_qualifier=""
            if ipkg == 'externals':
                possible_subset_qualifier=f', when="subset={self.full_umbrella}"'

            # now add additional deps:
            print('Generating externals:', ipkg)
            packages = self._load_packages(ipkg)
            print(f"{ipkg} packages: {packages}")
            for package in packages:
                if package.variant:
                    content += f'\n    depends_on("{package.name}@{package.version} {package.variant}"{possible_subset_qualifier})'
                else:
                    content += f'\n    depends_on("{package.name}@{package.version}"{possible_subset_qualifier})'

            self._write_package_file(ipkg, content)

        return

    def generate_daq_umbrella_package(self, repo_path, template_dir):
        repo_dir = os.path.join(repo_path, "spack-repo", "packages")
        template_dir = os.path.join(template_dir, "packages")
        ipkg = self.release_type
        itemp = os.path.join(template_dir, ipkg, 'package.py')
        if not os.path.exists(itemp):
            print(f"Error: template file {itemp} is not found!")
            return
        with open(itemp, 'r') as f:
            lines = f.read()
            lines = lines.replace("XRELEASEX", self.rdict["release"])
            lines = lines.replace("XTARGETX", self.full_umbrella)

        # now add additional deps:
        lines += '\n    for build_type in ["Debug", "RelWithDebInfo", "Release"]:'
        if self.release_type != "coredaq":
            lines += f'\n        depends_on(f"coredaq@{self.rdict["core_release"]} subset={self.full_umbrella} build_type={{build_type}} +dev", when=f"build_type={{build_type}} +dev")'
            lines += f'\n        depends_on(f"coredaq@{self.rdict["core_release"]} subset={self.full_umbrella} build_type={{build_type}} ~dev", when=f"build_type={{build_type}} ~dev")'
        for idep in self.rdict[ipkg]:
            iname = idep["name"]
            iver = idep["version"]
            if iname == "dbe":
                continue
            if iname.startswith("py-"):
                iver = idep["version"]
                lines += f'\n        depends_on(f"{iname}@{iver}")'
            else:
                # Nightlies
                if "daq" not in self.rdict["release"]:
                    iver = self.rdict["release"]
                lines += f'\n        depends_on(f"{iname}@{iver} build_type={{build_type}}", when=f"build_type={{build_type}}")'
        lines += '\n'

        ipkg_dir = os.path.join(repo_dir, ipkg)
        os.makedirs(ipkg_dir)
        ipkgpy = os.path.join(ipkg_dir, "package.py")
        with open(ipkgpy, 'w') as o:
            o.write(lines)
            print(f"Info: package.py has been written at {ipkgpy}.")
        return

    def generate_repo(self):
        self.repo_dir.mkdir(parents=True)
        self.update_package_metadata()
        self.write_release_yaml()
        self.generate_repo_file()
        self.generate_daq_packages()
        if self.release_type == "coredaq":
            self.generate_external_umbrella_package()
        #else:
        #    self.generate_daq_umbrella_package()
        return

    def generate_pypi_manifest(self, output_file):
        with open(output_file, 'w') as f:
            f.write("dune_pythonmodules=(\n")
            for i in self.rdict['pymodules']:
                iname = i["name"]
                iversion = i["version"]
                isource = i["source"]
                iline = f' "{iname}   {iversion}   {isource}"'
                f.write(iline + '\n')
            f.write(")\n")
        return

    def generate_pyvenv_requirements(self, output_path):
        output_file = Path(f"{output_path}/pyvenv_requirements.txt")
        pymodules = self.release_dict.get('pymodules')
        if not pymodules:
            raise ValueError("No pymodules found in release manifest.")

        with output_file.open('w') as f:
            for i in pymodules:
                package = DAQPackage(i)
                print('Package?', package)
                iname = i["name"]
                iversion = i["version"]
                if i["source"] == "pypi":
                    iline = f'{iname}=={iversion}'
                else:
                    iuser = i["source"].replace("github_", "")

                    if iversion == "develop" and not iname == "moo":
                        (itag, ihash) = get_commit_hash(iname, iversion, iversion)
                        iline = f"git+https://github.com/{iuser}/{iname}@{ihash}#egg={iname}"
                    elif iname == "moo":
                        iline = f"git+https://github.com/{iuser}/{iname}@{iversion}#egg={iname}"
                    else:
                        iline = f"git+https://github.com/{iuser}/{iname}@v{iversion}#egg={iname}"
                f.write(iline + '\n')
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
    parser.add_argument('--pypi-manifest', action='store_true',
                        help="whether to generate file containing bash array for python modules;")
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
        args.overwrite_daq_cmake
    )

    #if args.update_hash:
    #    daq_release.update_hashes()

    if args.pypi_manifest:
        #os.makedirs(args.output_path, exist_ok=True)
        #outfile = os.path.join(args.output_path, 'pypi_manifest.sh')
        daq_release.generate_pypi_manifest()
    elif args.pyvenv_requirements:
        #os.makedirs(args.output_path, exist_ok=True)
        #outfile = os.path.join(args.output_path, 'pyvenv_requirements.txt')
        daq_release.generate_pyvenv_requirements()
    #elif args.check_branch:
    #    tmp_dir = tempfile.mkdtemp()
    #    daq_release.copy_release_yaml(tmp_dir, True)
    #    shutil.rmtree(tmp_dir)
    else:
        if not args.release_name:
            raise ValueError("--release-name is required for generating a release repo.")
        if not args.template_path:
            raise ValueError("--template-path is required for generating a release repo.")
        daq_release.generate_repo()
        #daq_release.generate_repo(args.update_hash, args.core_release)
        #daq_release.generate_repo(args.output_path, args.template_path,
        #                          args.update_hash, args.release_name,
        #                          args.core_release)
