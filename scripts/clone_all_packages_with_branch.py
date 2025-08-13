import argparse
import os
import requests
import sys

from spack.dr_tools import get_packages

ORG = "DUNE-DAQ"

THIS_DIR=os.path.dirname( os.path.realpath(__file__) )
PACKAGE_GROUPS = ["coredaq", "fddaq", "nddaq"]
RELEASE_TYPE = "develop"


def main():
    GITHUB_TOKEN = os.getenv("GITHUB_TOKEN", None)

    if GITHUB_TOKEN is None:
        print("GITHUB_TOKEN environment variable is not set.")
        sys.exit(1)

    headers = {
        "Accept": "application/vnd.github+json",
        "Authorization": f"Bearer {GITHUB_TOKEN}"
    }
    parser = argparse.ArgumentParser(description="Get all repository in the DUNE-DAQ org which have a specific branch.")
    parser.add_argument("--branch", help="If set, only return packages from this branch", required=True)
    parser.add_argument("--py-modules", help="only do python repository, else do only c++", action="store_true")
    parser.add_argument("--verbose", action="store_true", help="Print more information about the packages")
    parser.add_argument("--package-group", choices=PACKAGE_GROUPS, default="coredaq", help="Package group to check. Default is 'coredaq'.")
    args = parser.parse_args()
    branch = args.branch
    verbose = args.verbose
    py = args.py_modules
    package_group = args.package_group

    repos_to_clone = []
    yaml_filename = "{}/../configs/{}/{}-{}/release.yaml".format(THIS_DIR, package_group, package_group, RELEASE_TYPE)
    print(yaml_filename)
    if not os.path.exists(yaml_filename):
        print(f"YAML file {yaml_filename} does not exist.")
        return
    if verbose:
        print(f"Checking {yaml_filename} for branch {branch} in package group {package_group}")

    for repo in get_packages(yaml_file=yaml_filename, package_group=package_group if not py else "pymodules"):

        branch_url = f"https://api.github.com/repos/{ORG}/{repo}/branches/{branch}"
        r = requests.get(branch_url, headers=headers)

        if r.status_code == 200:
            repos_to_clone += [repo]
            if verbose:
                print(f"\N{WHITE HEAVY CHECK MARK} Found \'{repo}\' on branch \'{branch}\'")
        elif r.status_code == 403:
            if verbose:
                print(f"\N{CROSS MARK} Access forbidden for repository \'{repo}\'. Error message: {r.json().get('message')}")
                break
        elif verbose:
            print(f"\N{CROSS MARK} Repository \'{repo}\' does not have branch \'{branch}\' (response code {r.status_code}, response text: {r.json().get('message')})")

    for repos in repos_to_clone:
        if verbose:
            print(f"Cloning {repos}")
        os.system(f"git clone https://github.com/{ORG}/{repos}.git -b {branch}")
if __name__ == "__main__":
    main()
