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

from mappings import cmake_to_spack, pyvenv_url_names
sys.path.append(str(Path(__file__).resolve().parent.parent))
from run_command import run_command

contains_oks_file = {}

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

def get_contains_oks_file_backup(repo_path_name):

    assert os.path.exists(repo_path_name), f"The {get_contains_oks_file.__name__} function is unable to find expected path {repo_path_name}"

    repo_path = pathlib.PosixPath(repo_path_name)

    for glob_extension in ["*.schema.xml", "*.data.xml"]:
        if len(list(repo_path.rglob(glob_extension))) > 0:
            return True

    return False

# TODO: This should be handled automatically
#def check_branch_exists(repo, branch):
#    command = f'git ls-remote --exit-code https://github.com/DUNE-DAQ/{repo}.git --heads origin {branch}'
#    args = command.split()
#    subproc = subprocess.run(args)
#    if subproc.returncode == 0:
#        return True
#    print(f'WARNING: No branch {branch} exists for package {repo}; defaulting to develop')
#    return False

@dataclass
class DAQPackage:
    name: str
    version: str
    commit: str = None
    source: str = None
    variant: str = None

    @classmethod
    def from_dict(cls, d: dict) -> DAQPackage:
        return cls(
            name    = d.get("name"),
            version = d.get("version"),
            commit  = d.get("commit"),
            source  = d.get("source"),
            variant = d.get("variant"),
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
        print(f'{self.name} is pymodule?', self.is_pymodule)
        print(f'{self.name} is dunedaq pymodule?', self.is_dunedaq_pymodule)
        print(f'{self.name} source:', self.source)
        print(f'{self.name} source type:', type(self.source))
        if self.is_pymodule and not self.is_dunedaq_pymodule:
            return None
        return f"https://github.com/DUNE-DAQ/{self.name}"

    def contains_oks_file(self, ref="HEAD") -> bool:
        print('REPO URL:', self.repo_url)
        if not self.repo_url:
            return False
        with tempfile.TemporaryDirectory() as tmpdir:
            repo = Repo.init(tmpdir, bare=True)
            repo.git.fetch(self.repo_url, ref, depth=1)
            commit = repo.commit("FETCH_HEAD")

            for blob in commit.tree.traverse():
                if blob.type == "blob" and blob.path.endswith((".schema.xml", ".data.xml")):
                    return True

        return False

    def update_commit_hash(self, fall_back_tag="develop"):
        if self.source == "pypi":
            return
        # Necessary bespoke logic for the singular case of elisa-client-api >:[
        url_name = self.name if self.name != "elisa-client-api" else "elisa_client_api"
        url = f"https://github.com/DUNE-DAQ/{url_name}"

        # Pymodules don't have a "v" in the manifest versions since 
        # they need to be pip-installed, but we tag them with the 
        # "v" on GitHub
        version = self.version
        if self.is_dunedaq_pymodule and self.version_is_tag:
            version = f"v{version}"
        
        result = run_command(f"git ls-remote {url} {version} | cut -f 1 | cut -c1-7")
        print(f'Got hash {result["stdout"]} for {self.name} @ {version}')
        self.commit = result['stdout']

    def get_file(self):
        # Download file from package
        pass


class DAQRelease:
    def __init__(
        self,
        release_dict: dict,
        overwrite_branch: str = "",
        overwrite_daq_cmake: bool = False,
    ):
        self.release_dict = release_dict
        self.overwrite_branch = overwrite_branch
        self.overwrite_daq_cmake = overwrite_daq_cmake

        self.rtype = self.get_release_type()
        self.packages = self.load_packages()

        # TODO: Edit elsewhere to simply use self.release_dict['umbrella]
        self.full_umbrella = self.release_dict['umbrella']

    @classmethod
    def from_yaml(
        cls,
        release_dict: dict,
        overwrite_branch: str = "",
        overwrite_daq_cmake: bool = False,
    ) -> DAQRelease:

        return cls(
            release_dict=release_dict,
            overwrite_branch=overwrite_branch,
            overwrite_daq_cmake=overwrite_daq_cmake,
        )
    
    def get_release_type(self):
        return self.release_dict['type']

    def load_packages(self):
        package_list = [
            DAQPackage.from_dict(entry)
            for entry in self.release_dict.get(self.rtype)
        ]

        if "pymodules" in self.release_dict:
            pymodules = [
                DAQPackage.from_dict(entry)
                for entry in self.release_dict.get("pymodules")
                #if entry.get("source") == "github_DUNE-DAQ"
            ]
            package_list.extend(pymodules)

        return package_list

    def update_hashes(self):
        for package in self.packages:
            package.update_commit_hash()

    def set_release(self, release_name, core_release=""):
        if core_release != "":
            self.rdict["core_release"] = core_release
        self.rdict["release"] = release_name

    def write_release_yaml(self, repo_path):
        repo_dir = Path(f"{repo_path}/spack-repo")
        repo_dir.mkdir(parents=True)

        output_file = Path(f"{repo_dir}/{self.release_dict['release']}.yaml")
        with output_file.open("w") as outfile:
            outfile.write('---\n')
            yaml.dump(self.release_dict, outfile, Dumper=MyDumper, default_flow_style=False, sort_keys=False)
        return

    def get_file_from_package(self, package_name, branch_name, file_name):
        if self.overwrite_branch != '':
            if check_branch_exists(package_name, self.overwrite_branch):
                branch_name = self.overwrite_branch
        file_url = f'https://raw.githubusercontent.com/DUNE-DAQ/{package_name}/{branch_name}/{file_name}'
        command = f'curl -o {file_name} --fail {file_url}'
        run_command(command)
        #check_output(command, 5)

    def get_cmake_dependencies(self, package_name, branch_name):

        file_name = "CMakeLists.txt"
        self.get_file_from_package(package_name, branch_name, file_name)

        cmake_dependencies_list = []
        with open(file_name, 'r') as infile:
            lines = infile.read()
            # Parse package names from find_package calls. Everything up to the first
            # white space character will be taken as the package name (i.e., no "REQUIRED"
            # or "COMPONENTS"
            find_package_pattern  = re.compile(r'\s*[^# ]\s*find_package\(\s*([^)\s]+)')
            cmake_dependencies_list = find_package_pattern.findall(lines)
            # Special cases where the dependency has no explicit find_package call
            find_daq_codegen = re.search(r'\s*[^# ]\s*daq_codegen\(', lines)
            if find_daq_codegen:
                cmake_dependencies_list.append('py-moo')
            find_pybind = re.search(r'\s*[^# ]\s*daq_add_python_bindings\(', lines)
            if find_pybind: 
                cmake_dependencies_list.append('pybind11')
            find_numa = re.search(r'\s*[^# ]\s*pkg_check_modules\(numa', lines)
            if find_numa:
                cmake_dependencies_list.append('numactl')
        cmake_dependencies_list = [dep.lower() for dep in cmake_dependencies_list]
        return cmake_dependencies_list

    def generate_depends_on_list(self, cmake_package_list):
        depends_on_list = ""
        for idep in cmake_package_list:
            # Special cases where find_package call in CMakeLists is not sufficient
            if idep in cmake_to_spack:
                idep = cmake_to_spack[idep]
            depends_on_list += f'\n    depends_on("{idep}")'
        return depends_on_list

    def generate_repo_file(self, repo_path):
        repo_dir = os.path.join(repo_path, "spack-repo")
        os.makedirs(repo_dir, exist_ok=True)
        with open(os.path.join(repo_dir, "repo.yaml"), 'w') as f:
            repo_string = "repo:\n  namespace: '{}'\n".format(
                self.rdict["release"])
            f.write(repo_string)
        return

    def generate_daq_package(self, repo_path, template_dir):
        repo_dir = os.path.join(repo_path, "spack-repo", "packages")
        template_dir = os.path.join(template_dir, "packages")
        for ipkg in self.rdict[self.rtype]:
            itemp = os.path.join(template_dir, ipkg["name"], 'package.py')
            if not os.path.exists(itemp):
                print(f"Error: template file {itemp} is not found!")
                continue
            with open(itemp, 'r') as f:
                lines = f.read()
                # Nightlies
                if "daq" not in self.rdict["release"]:
                    lines = lines.replace("XVERSIONX", self.rdict["release"])
                # Stable release
                else:
                    lines = lines.replace("XVERSIONX", ipkg["version"])
                if ipkg["commit"] is not None:
                    lines = lines.replace("XHASHX", ipkg["commit"])
                # Infer dependencies from CMakeLists.txt
                cmake_package_list = self.get_cmake_dependencies(ipkg["name"], ipkg["commit"])
                depends_on_list = self.generate_depends_on_list(cmake_package_list)
                lines = lines.replace("XDEPENDSX", depends_on_list)

                if contains_oks_file[ ipkg["name"] ]:
                    lines = lines.replace("XDBPATHX", "env.prepend_path(\"DUNEDAQ_DB_PATH\", self.prefix + \"/share\")")
                else:
                    lines = lines.replace("XDBPATHX", "")

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
        ipkg = self.rtype
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
        if self.rtype != "coredaq":
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
        if self.rtype == "coredaq":
            self.generate_external_umbrella_package(repo_path, template_dir)
        self.generate_daq_umbrella_package(repo_path, template_dir)
        return

    def generate_repo(self, outdir, tempdir, update_hash, release_name, core_release):
        if release_name is not None:
            self.set_release(release_name, core_release)
        #self.copy_release_yaml(outdir, update_hash)
        if update_hash:
            self.update_hashes()
        self.write_release_yaml(outdir)
        self.generate_repo_file(outdir)
        self.generate_daq_package(outdir, tempdir)
        self.generate_umbrella_package(outdir, tempdir)
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

    def generate_pyvenv_requirements(self, output_file):
        with open(output_file, 'w') as f:
            for i in self.rdict['pymodules']:
                iname = i["name"]
                iversion = i["version"]
                if i["source"] == "pypi":
                    iline = f'{iname}=={iversion}'
                if i["source"].startswith("github"):
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
                        default="../../spack-repos/packages",
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
    parser.add_argument('-o', '--output-path',
                        help="path to the generated spack repo;")
    parser.add_argument('--pypi-manifest', action='store_true',
                        help="whether to generate file containing bash array for python modules;")
    parser.add_argument('--pyvenv-requirements', action='store_true',
                        help="whether to generate requirements file for pyvenv;")
    parser.add_argument('--core-release',
                        help="core release name")

    args = parser.parse_args()

    #daq_release = DAQRelease(args.input_manifest, args.overwrite_branch, args.overwrite_daq_cmake)
    release_dict = load_release_data(args.input_manifest)
    daq_release = DAQRelease.from_yaml(release_dict, args.overwrite_branch, args.overwrite_daq_cmake)
    if args.update_hash:
        daq_release.update_hashes()

    if args.pypi_manifest:
        os.makedirs(args.output_path, exist_ok=True)
        outfile = os.path.join(args.output_path, 'pypi_manifest.sh')
        daq_release.generate_pypi_manifest(outfile)
    elif args.pyvenv_requirements:
        os.makedirs(args.output_path, exist_ok=True)
        outfile = os.path.join(args.output_path, 'pyvenv_requirements.txt')
        daq_release.generate_pyvenv_requirements(outfile)
    #elif args.check_branch:
    #    tmp_dir = tempfile.mkdtemp()
    #    daq_release.copy_release_yaml(tmp_dir, True)
    #    shutil.rmtree(tmp_dir)
    else:
        daq_release.generate_repo(args.output_path, args.template_path,
                                  args.update_hash, args.release_name,
                                  args.core_release)
