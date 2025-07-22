#!/usr/bin/env python3

import os
import yaml
import argparse
import subprocess
import re
import textwrap

from spack.dr_tools import parse_yaml_file
from spack.mappings import pymodule_github_url_names

def check_output(cmd, is_success_required=True):
    irun = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    stdout, stderr = irun.communicate()
    rc = irun.returncode
    if rc != 0:
        if is_success_required:
            print(f'\nERROR: command "{cmd}" failed with exit code {rc}')
            print(f'STDOUT:\n{stdout.decode()}')
            print(f'STDERR:\n{stderr.decode()}')

            exit(10)
        else:
            print('Non-zero exit status from checkout attempt; this is acceptable')
    else:
        print("Checkout successful")

def checkout_commit(repo, commit, outdir, is_success_required=True):
    if repo in pymodule_github_url_names:
        repo = pymodule_github_url_names[repo]
    cmd = textwrap.dedent(f"""
        mkdir -p {outdir} && cd {outdir} &&
        git clone https://github.com/DUNE-DAQ/{repo}.git; 
        cd {repo}; 
        git checkout {commit}
    """)
    print(f"\nInfo: attempting checkout of {repo:<20} {commit:<20} under {outdir}")
    check_output(cmd, is_success_required)
    return

def checkout_tag(repo, version, outdir, is_pymodule=False):
    if repo in pymodule_github_url_names:
        repo = pymodule_github_url_names[repo]
    cmd = textwrap.dedent(f"""
        mkdir -p {outdir} && cd {outdir} &&
        git clone https://github.com/DUNE-DAQ/{repo}.git &&
        cd {repo} &&
        git fetch --tags &&
        if ! git show-ref --tags --verify --quiet "refs/tags/{version}"; then
            echo "The tag {version} does not exist for package {repo}. Exiting..." && exit 1
        fi
        git checkout {version} &&
        if [[ "{is_pymodule}" == "False" ]]; then
            cmake_version=`grep "^project" CMakeLists.txt |grep ")$"|grep -oP "(([[:digit:]]+\.)([[:digit:]]+\.)([[:digit:]]+))"` &&
            tag=v"$cmake_version" &&
            echo $tag &&
            echo $version &&
            if [[ $tag != "{version}" ]]; then
                echo "Tag mismatches version in CMakeLists.txt ( $tag vs {version} )" && exit 1;
            fi
        # TODO AJM 2025/07/08: Once python package structure is standardized, add an else block for checking python package tags
        fi
    """)
    check_output(cmd)
    print(f"Info: verified version in CMake, checked out {repo:<20} {version:<20} under {outdir}.")
    return

def load_packages(manifest_path, load_pymodules=False):
    yaml_dict = parse_yaml_file(manifest_path)
    pkgs = yaml_dict.get("coredaq", []) + yaml_dict.get("fddaq", []) + yaml_dict.get("nddaq", [])
    if load_pymodules:
        pymodules = yaml_dict.get("pymodules", [])
        if not pymodules:
            print(f'WARNING: You\'ve requested --pymodules, but {manifest_path} does not have any pymodules.')
        dunedaq_pymodules = [entry for entry in pymodules if entry["source"] == "github_DUNE-DAQ"]
        pkgs += dunedaq_pymodules
    if not pkgs:
        print("Error: No packages found in manifest.")
        exit(20)
    return pkgs

def get_checkout_token(name, branch, commit, version, source, check_tag=False):
    if name == 'daq-cmake' and branch:
        return version
    if branch and re.search(r"\d+\.\d+\.\d+", version):
        print(textwrap.dedent(f"""\n
            Error: {name} uses a fixed tag in the manifest. Can't override with a branch.
        """))
        exit(30)
    if version == 'develop' and check_tag:
        print(textwrap.dedent(f"""\n
            Error: You requested to check tags ('-c'), but the manifest specifies the develop branch of {name}.
        """))
        exit(31)
    if branch:
        return branch
    if re.search(r"\d+\.\d+\.\d+", version):
        # Python package versions don't start with "v" in the release manifest; 
        # add it here for consistency in checkout tokens
        return f"v{version}" if source else version
    if commit:
        return commit
    return "develop"


if __name__ == "__main__":
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

    args = parser.parse_args()

    pkgs = load_packages(args.input_manifest, args.pymodules)

    if args.all_packages:
        for pkg in pkgs:
            name = pkg.get("name")
            if name == "elisa-client-api":
                name = "elisa_client_api"
            commit = pkg.get("commit")
            version = pkg.get("version")
            source = pkg.get("source")
            is_pymodule = bool(source)
            token = get_checkout_token(name, args.branch, commit, version, source, args.check_tag)
            if args.check_tag:
                checkout_tag(name, token, args.output_path, is_pymodule)
            else:
                if name == 'daq-cmake' and args.branch:
                    checkout_commit(name, token, args.output_path, is_success_required=False)
                else:
                    checkout_commit(name, token, args.output_path) 
    elif args.package is not None:
        pkg_entry = next((pkg for pkg in pkgs if pkg.get("name") == args.package), None)
        if not pkg_entry:
            print(f"Error: {args.package} not found in {args.input_manifest}")
            print(f"       Note that you must pass -m or --pymodules to load python packages.")
            exit(21)
        name = pkg_entry.get("name")
        if name == "elisa-client-api":
            name = "elisa_client_api"
        commit = pkg_entry.get("commit")
        version = pkg_entry.get("version")
        source = pkg_entry.get("source")
        is_pymodule = bool(source)
        token = get_checkout_token(name, args.branch, commit, version, source, args.check_tag)
        if args.check_tag:
            checkout_tag(args.package, token, args.output_path, is_pymodule)
        else:
            checkout_commit(args.package, token, args.output_path)
    else:
        print('Error: please specify "-a" or "-p <pkg>" option.')
        exit(22)
