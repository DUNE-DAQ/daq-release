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
    commit: str = None
    source: str = None
    variant: str = None

    contains_oks_file: bool = False
    cmake_dependencies: list[str] = field(default_factory=list)

    # Keep a reference to the original data in case mutations are necessary
    _raw: dict | None = field(default=None, repr=False)

    def __post_init__(self):
        if not isinstance(self.version, str):
            self.version = str(self.version)

    @classmethod
    def from_dict(cls, d: dict) -> DAQPackage:
        return cls(
            name    = d.get("name"),
            version = d.get("version"),
            commit  = d.get("commit"),
            source  = d.get("source"),
            variant = d.get("variant"),
            _raw    = d,
        )

    @property
    def is_pymodule(self) -> bool:
        return bool(self.source)

    @property
    def is_dunedaq_pymodule(self) -> bool:
        return bool(self.source == "github_DUNE-DAQ")

    @property
    def version_is_tag(self) -> bool:
        return bool(re.search('\d+.\d+.\d+', self.version))

    @property
    def repo_url(self) -> str:
        if self.is_pymodule and not self.is_dunedaq_pymodule:
            return None
        return f"https://github.com/DUNE-DAQ/{self.name}"

    # Update commit in both the DAQPackage object and the upstream release dict
    def set_commit(self, commit: str):
        self.commit = commit
        self._raw["commit"] = commit

    def update_commit_hash(self, repo_path_name, fall_back_tag="develop"):
        if self.is_pymodule and not self.is_dunedaq_pymodule:
            return
        
        result = run_command(f"git rev-parse --short HEAD", cwd=repo_path_name)
        self.set_commit(result['stdout'])

    def get_contains_oks_file(self, repo_path_name):
        assert os.path.exists(repo_path_name), f"The {get_contains_oks_file.__name__} function is unable to find expected path {repo_path_name}"

        repo_path = pathlib.PosixPath(repo_path_name)

        for glob_extension in ["*.schema.xml", "*.data.xml"]:
            if len(list(repo_path.rglob(glob_extension))) > 0:
                self.contains_oks_file = True

    def get_cmake_dependencies(self, repo_path_name: str):
        cmakelists = Path(f"{repo_path_name}/CMakeLists.txt")
        if not cmakelists.is_file():
            return

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
            print('Update hash?', update_package_hash)
            if update_package_hash:
                self.update_commit_hash(tmpdir)
            self.get_contains_oks_file(tmpdir)
            self.get_cmake_dependencies(tmpdir)

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

        if self.release_name is not None:
            self.release_dict["release"] = self.release_name

        if self.core_release:
            self.release_dict["core_release"] = self.core_release

        self.packages = self.load_packages()

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
    def template_dir(self):
        return Path(self.template_path) / "packages"

    @property
    def is_nightly_release(self):
        return "daq" not in self.release_dict["release"]

    def load_packages(self):
        package_list = [
            DAQPackage.from_dict(entry)
            for entry in self.release_dict.get(self.release_type)
        ]

        if "pymodules" in self.release_dict:
            pymodules = [
                DAQPackage.from_dict(entry)
                for entry in self.release_dict.get("pymodules")
                if entry.get("source") == "github_DUNE-DAQ"
            ]
            package_list.extend(pymodules)

        return package_list

    def update_package_metadata(self):
        for package in self.packages:
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

    def generate_daq_packages(self):
        #for ipkg in self.release_dict[self.release_type]:
        for package in self.packages:
            if package.is_pymodule: continue
        
            #itemp = os.path.join(template_dir, ipkg["name"], 'package.py')
            package_template = Path(self.template_dir) / package.name / 'package.py'
            if not package_template.is_file():
                print(f"WARNING: template file {package_template} is not found! No package.py will be generated.")
                continue
            print(f'Template file {package_template} found.')

            content = package_template.read_text()

            if self.is_nightly_release:
                content = content.replace("XVERSIONX", self.release_dict["release"])
            else:
                content = content.replace("XVERSIONX", package.version)

            content = content.replace("XHASHX", package.commit)

            depends_on_list = [f'depends_on("{dep}")' for dep in package.cmake_dependencies]
            content = content.replace("XDEPENDSX", "\n    ".join(depends_on_list))

            dbpath = (
                'env.prepend_path("DUNEDAQ_DB_PATH", self.prefix + "/share")'
                if package.contains_oks_file
                else ""
            )
            content = content.replace("XDBPATHX", dbpath)
            print('Final content:', content)
            generated_package_file = Path(self.repo_dir) / package.name / "package.py"
            generated_package_file.parent.mkdir(parents=True)
            generated_package_file.write_text(content)

    def temp(self):
        ipkg_dir = os.path.join(repo_dir, ipkg["name"])
        os.makedirs(ipkg_dir)
        ipkgpy = os.path.join(ipkg_dir, "package.py")
        with open(ipkgpy, 'w') as o:
            o.write(lines)
            print(f"Info: package.py has been written at {ipkgpy}.")
        return

    def generate_external_umbrella_package(self, repo_path, template_dir):
        repo_dir = os.path.join(repo_path, "spack-repo", "packages")
        template_dir = os.path.join(template_dir, "packages")

        for ipkg in ['devtools', 'externals', 'systems']:
            itemp = os.path.join(template_dir, ipkg, 'package.py')
            if not os.path.exists(itemp):
                print(f"Error: template file {itemp} is not found!")
                continue
            with open(itemp, 'r') as f:
                lines = f.read()
                lines = lines.replace("XRELEASEX", self.rdict["release"])
                lines = lines.replace("XTARGETX", self.full_umbrella)

            possible_subset_qualifier=""
            if ipkg == 'externals':
                possible_subset_qualifier=f', when="subset={self.full_umbrella}"'

            # now add additional deps:
            for idep in self.rdict[ipkg]:
                iname = idep["name"]
                iver = idep["version"]
                # Externals, system/devtools etc, variant is used instead of
                # version
                ivar = idep["variant"]
                if ivar == None:
                    lines += f'\n    depends_on("{iname}@{iver}"{possible_subset_qualifier})'
                else:
                    lines += f'\n    depends_on("{iname}@{iver} {ivar}"{possible_subset_qualifier})'
            lines += '\n'
            ipkg_dir = os.path.join(repo_dir, ipkg)
            os.makedirs(ipkg_dir)
            ipkgpy = os.path.join(ipkg_dir, "package.py")
            with open(ipkgpy, 'w') as o:
                o.write(lines)
                print(f"Info: package.py has been written at {ipkgpy}.")
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

    def generate_umbrella_package(self, repo_path, template_dir):
        if self.release_type == "coredaq":
            self.generate_external_umbrella_package(repo_path, template_dir)
        self.generate_daq_umbrella_package(repo_path, template_dir)
        return

    def generate_repo(self):
        self.repo_dir.mkdir(parents=True)
        self.update_package_metadata()
        self.write_release_yaml()
        self.generate_repo_file()
        self.generate_daq_packages()
        #self.generate_umbrella_package()
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
