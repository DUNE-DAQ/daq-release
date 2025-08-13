import argparse
import os
import requests
import sys

from spack.dr_tools import get_packages

ORG = "DUNE-DAQ"

THIS_DIR=os.path.dirname( os.path.realpath(__file__) )
PACKAGE_GROUPS = ["coredaq", "fddaq", "pymodules"]
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
    parser.add_argument("--verbose", action="store_true", help="Print more information about the packages")
    parser.add_argument("--package-group", choices=PACKAGE_GROUPS, help="Package group to check. Can be 'coredaq', 'fddaq', or 'pymodules'")
    args = parser.parse_args()
    branch = args.branch
    verbose = args.verbose
    package_group = args.package_group

    if package_group not in ["coredaq", "fddaq", "pymodules"]:
        print(f"ERROR: package group \"{package_group}\" not known")
        sys.exit(2)

    repos_to_clone = []

    # Account for the fact that for historical reasons the DUNE DAQ Python packages are listed in
    # the release.yaml's in the fddaq directory

    if package_group != "pymodules":
        yaml_filename = "{}/../configs/{}/{}-{}/release.yaml".format(THIS_DIR, package_group, package_group, RELEASE_TYPE)
    else:
        yaml_filename = "{}/../configs/fddaq/fddaq-{}/release.yaml".format(THIS_DIR, RELEASE_TYPE)

    print(yaml_filename)
    if not os.path.exists(yaml_filename):
        print(f"YAML file {yaml_filename} does not exist.")
        return
    if verbose:
        print(f"Checking {yaml_filename} for branch {branch} in package group {package_group}")

    for repo in get_packages(yaml_file=yaml_filename, package_group=package_group):

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
