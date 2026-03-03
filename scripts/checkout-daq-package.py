#!/usr/bin/env python3

import os, sys
import yaml
import argparse
import subprocess
import re
import textwrap
from pathlib import Path

from spack.dr_tools import parse_yaml_file
from run_command import run_command


class DAQCheckoutArea:

    def __init__(self, 
                 release_manifest, path, package="", all_packages=False, 
                 branch="", load_pymodules=False, check_tag=False, 
                 overwrite=False, continue_on_error=False):
        if package and all_packages:
            raise ValueError("Cannot specify both '-a' (all packages) and '-p' (single package).")
        if not package and not all_packages:
            raise ValueError("Please specify '-a' or '-p <pkg>' option.")

        self.release_manifest = release_manifest
        self.path = Path(path)
        self.package = package
        self.all_packages = all_packages
        self.load_pymodules = load_pymodules
        self.config = {
            "branch": branch,
            "check_tag": check_tag,
            "overwrite": overwrite,
            "continue_on_error": continue_on_error
        }
        self.packages_with_errors = []

        self.package_list = self.load_packages()
        if package:
            pkg_entry = next((pkg for pkg in self.package_list if pkg.get("name") == self.package), None)
            if not pkg_entry:
                raise ValueError(textwrap.dedent(f"""
                    Package {self.package} not found in {self.release_manifest}
                    Note that you must pass '-m' or '--pymodules' to load python packages."""))
            self.package_list = [pkg_entry]

        if not self.path.is_dir():
            self.path.mkdir(parents=True)
    
    def load_packages(self):
        yaml_dict = parse_yaml_file(self.release_manifest)
        pkgs = yaml_dict.get("coredaq", []) + yaml_dict.get("fddaq", []) + yaml_dict.get("nddaq", [])
        if self.load_pymodules:
            pymodules = yaml_dict.get("pymodules", [])
            if not pymodules:
                raise ValueError(f'WARNING: You\'ve requested --pymodules, but {self.release_manifest} does not have any pymodules.')
            dunedaq_pymodules = [entry for entry in pymodules if entry["source"] == "github_DUNE-DAQ"]
            pkgs = dunedaq_pymodules
        if not pkgs:
            raise ValueError(f"No packages loaded from {self.release_manifest}.")
        return pkgs

    def confirm_overwrite(self):
        if not self.config["overwrite"]:
            return
        target = " ".join(pkg["name"] for pkg in self.package_list)
        packages_to_overwrite = [tgt for tgt in target.split() if Path(f"{self.path}/{tgt}").is_dir()]
        if not packages_to_overwrite:
            return
        proceed = input(f"This action will overwrite {packages_to_overwrite} in {self.path}. Continue? [y/N] ")
        if proceed.lower() not in ("y", "yes"):
            raise RuntimeError("Aborted")

    def checkout_packages(self):
        for package_dict in self.package_list:
            checkout_package = DAQCheckoutPackage(package_dict, self)
            checkout_package.checkout()
            if self.config["check_tag"]:
                checkout_package.verify_tags()
            if not checkout_package.success:
                print(f"There was a problem with {checkout_package.name}")
                self.packages_with_errors.append(checkout_package.name)

class DAQCheckoutPackage:

    def __init__(self, package_dict, checkout_area):
        self.name = package_dict.get("name")
        # AJM Nov. 17, 2025: for now, keep next two lines for backwards compatibility
        if self.name == "elisa-client-api":
            self.name = "elisa_client_api"
        self.commit = package_dict.get("commit")
        self.version = package_dict.get("version")
        self.source = package_dict.get("source")
        # Only python packages have a "source" field
        self.is_pymodule = bool(package_dict.get("source"))
        self.checkout_area = Path(f"{checkout_area.path}")
        self.checkout_path = Path(f"{checkout_area.path}/{self.name}")
        self.branch = checkout_area.config["branch"]
        self.check_tag = checkout_area.config["check_tag"]
        self.overwrite = checkout_area.config["overwrite"]
        self.continue_on_error = checkout_area.config["continue_on_error"]
        self.checkout_token = self.get_checkout_token()
        self.success = True

    def get_checkout_token(self):
        if self.name == 'daq-cmake' and self.branch:
            return self.version
        if self.branch and (self.commit or re.search(r"\d+\.\d+\.\d+", self.version)):
            raise ValueError(f"{self.name} uses a fixed tag or commit in the manifest. Can't override with a branch.")
        if self.version == 'develop' and self.check_tag:
            raise ValueError(f"Error: You requested to check tags ('-c'), but the manifest specifies the develop branch of {self.name}.")
        if self.branch:
            return self.branch
        if re.search(r"\d+\.\d+\.\d+", self.version):
            # Python package versions don't start with "v" in the release manifest; 
            # add it here for consistency in checkout tokens
            return f"v{self.version}" if self.source else self.version
        if self.commit:
            return self.commit
        return "develop"

    def checkout(self):
        if self.overwrite and self.checkout_path.is_dir():
            run_command(f"rm -rf {self.checkout_path}", cwd=self.checkout_area)

        run_command(f"git clone https://github.com/DUNE-DAQ/{self.name}.git", cwd=self.checkout_area)
        print(f"\nINFO: Attempting checkout of {self.name:<20} {self.checkout_token:<20} under {self.checkout_path}")
        checkout_results = run_command(f"git checkout {self.checkout_token}", cwd=self.checkout_path, continue_on_error=self.continue_on_error)
        if checkout_results["exit_code"] != 0:
            self.success = False

    def verify_tags(self):
        cmakelists_tag = self.get_cmakelists_version()
        pyproj_tag = self.get_pyproj_version()

        if self.is_pymodule:
            self.version = f"v{self.version}"

        # Verify tag on GitHub
        run_command(["git", "fetch", "--tags"], cwd=self.checkout_path)
        run_command(["git", "show-ref", "--tags", "--verify", "--quiet", f"refs/tags/{self.version}"], 
                           cwd=self.checkout_path, context=f"Tag {self.version} does not exist for package {self.name}",
                           continue_on_error=self.continue_on_error)

        # Verify tag in CMakeLists and/or pyproject.toml
        if cmakelists_tag is not None and f"v{cmakelists_tag}" != self.version:
            self.success = False
            if not self.continue_on_error:
                raise RuntimeError(f"Tag mismatch in {self.name}: you requested {self.version}, but the CMakeLists has {cmakelists_tag}")

        if pyproj_tag is not None and f"v{pyproj_tag}" != self.version:
            self.success = False
            if not self.continue_on_error:
                raise RuntimeError(f"Tag mismatch in {self.name}: you requested {self.version}, but the pyproject.toml has {pyproj_tag}")

    def get_cmakelists_version(self):
        cmakelists_file = Path(self.checkout_path) / "CMakeLists.txt"
        if not cmakelists_file.exists():
            return None

        version_regex = re.compile(r'project\s*\(\S*\w+\s+VERSION\s+(\d+\.\d+\.\d+)')

        with cmakelists_file.open() as f:
            content = f.read()
        
        match = version_regex.search(content)
        if not match:
            print(f"WARNING: {self.checkout_path}: No CMakeLists version extracted")
            return None
        return match.group(1)

    def get_pyproj_version(self):
        pyproject_file = self.checkout_path / "pyproject.toml"
        if not pyproject_file.exists():
            return None

        version_regex = re.compile(r'^\s*version\s*=\s*["\']([^"\']+)["\']', re.MULTILINE)

        with pyproject_file.open() as f:
            content = f.read()

        # Match [project] section until the next [section] or end of file
        project_section = re.search(r'^\[project\]\s*(.*?)(?=^\[|\Z)', content, re.DOTALL | re.MULTILINE)
        if project_section:
            section_text = project_section.group(1)
            match = version_regex.search(section_text)
            if match:
                return match.group(1)
            else:
                print(f"{self.checkout_path}: No version found in [project] section")
                return None
        else:
            print(f"{self.checkout_path}: No [project] section found in pyproject.toml")
            return None
def main():
    parser = argparse.ArgumentParser(
        prog='checkout-daq-package.py',
        description="Tool for checking out DAQ package(s).",
        epilog="Questions and comments to jcfree@fnal.gov",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-p', '--package', default=None,
                        help='''DAQ package to checkout;''')
    parser.add_argument('-b', '--branch', default=None,
                        help='''branch name, tag name, or commit hash; the last two only to be used with the -p option for single package checkout''')
    parser.add_argument('-i', '--input-manifest', required=True,
                        help="path to the release manifest file;")
    parser.add_argument('-a', '--all-packages', action='store_true',
                        help="whether to checkout all DAQ pacakges;")
    parser.add_argument('-c', '--check-tag', action='store_true',
                        help="whether to check if tag is the same as used in CMakeLists.txt;")
    parser.add_argument('-o', '--output-path', default="./sourcecode",
                        help="path to the output directory;")
    parser.add_argument('-m', '--pymodules', action='store_true',
                        help="whether to checkout DAQ python modules;")
    parser.add_argument('--overwrite', action='store_true',
                        help="whether to overwrite the output directory if it already exists;")
    parser.add_argument('--continue-on-error', action='store_true',
                        help="whether to continue after a failed checkout or tag/commit verification;")

    args = parser.parse_args()

    checkout_area = DAQCheckoutArea(
        args.input_manifest, args.output_path, args.package, 
        args.all_packages, args.branch, args.pymodules, 
        args.check_tag, args.overwrite, args.continue_on_error
    )

    checkout_area.load_packages()
    checkout_area.confirm_overwrite()
    checkout_area.checkout_packages()

    success_count = len(checkout_area.package_list) - len(checkout_area.packages_with_errors)
    print("Successfully checked out", success_count ,"packages to", checkout_area.path)
    if checkout_area.packages_with_errors:
        print("The following packages had one or more errors:", checkout_area.packages_with_errors)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
