#!/usr/bin/env python3

import os
import yaml
import argparse
import subprocess
import re

def parse_yaml_file(fname):
    if not os.path.exists(fname):
        print("Error: -- YAML file {} does not exist".format(fname))
        exit(20)
    fman = ""
    with open(fname, 'r') as stream:
        try:
            fman = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
    return fman


def check_output(cmd, is_success_required = True):
    irun = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out = irun.communicate()
    rc = irun.returncode
    if rc != 0:
        if is_success_required:
            print('\nERROR: command "{}" has exit non-zero exit status, \
please check!\n'.format(cmd))
            print('Command output:\n {}\n'.format(out[0].decode('utf-8')))
            print('Command error:\n{}\n'.format(out[1].decode('utf-8')))

            exit(10)
        else:
            print('Non-zero exit status from checkout attempt; this is acceptable')
    else:
        print("Checkout successful")

def checkout_commit(repo, commit, outdir, is_success_required):
    cmd = f"""\nmkdir -p {outdir}; cd {outdir}; 
git clone https://github.com/DUNE-DAQ/{repo}.git; 
cd {repo}; 
git checkout {commit}
    """
    print(f"\nInfo: attempting checkout of {repo:<20} {commit:<20} under {outdir}")
    check_output(cmd, is_success_required)
    return

def checkout_tag(repo, commit, outdir):
    cmd = f"""\nmkdir -p {outdir}; cd {outdir}; \
git clone https://github.com/DUNE-DAQ/{repo}.git; \
cd {repo}; \
git checkout {commit}; \
cmake_version=`grep "^project" CMakeLists.txt |grep ")$"|grep -oP "(([[:digit:]]+\.)([[:digit:]]+\.)([[:digit:]]+))"`; \
tag=v"$cmake_version"; \
echo $tag ;\
echo $commit; \
if [[ $tag != "{commit}" ]]; then echo "Tag mismatches version in CMakeLists.txt ( $tag vs {commit} )" && exit 1; fi
    """
    check_output(cmd)
    print(f"Info: verified version in CMake, checked out {repo:<20} {commit:<20} under {outdir}.")
    return


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

    args = parser.parse_args()

    pkgs = []
    if args.input_manifest is not None:
        yaml_dict = parse_yaml_file(args.input_manifest)
        if "dunedaq" in yaml_dict:
            pkgs = parse_yaml_file(args.input_manifest)["dunedaq"]
        if "fddaq" in yaml_dict:
            pkgs = parse_yaml_file(args.input_manifest)["fddaq"]
        if "nddaq" in yaml_dict:
            pkgs = parse_yaml_file(args.input_manifest)["nddaq"]
    if len(pkgs) == 0:
        print("Error: proper release manifest file is required.")
        exit(20)

    if args.all_packages:
        for i in pkgs:
            if i["name"].startswith("py-"):
                continue
            if args.check_tag:
                checkout_tag(i["name"], i["version"], args.output_path)
            else:
                if args.branch is None:
                    checkout_token = None
                    if i["commit"] is not None:
                        checkout_token = i["commit"]
                    elif re.search(r"v[0-9]+\.[0-9]+\.[0-9]+", i["version"]):
                        checkout_token = i["version"]
                    else:
                        checkout_token = "develop"

                    checkout_commit(i["name"], checkout_token, args.output_path, is_success_required = True)
                else:
                    if i["commit"] is not None or re.search(r"v[0-9]+\.[0-9]+\.[0-9]+", i["version"]):
                        print(f'\nError: package {i["name"]} is listed in {args.input_manifest}\nwith a commit hash and/or version; can\'t use the branch override\nargument to the script in this case')
                        exit(30)
                    checkout_commit(i["name"], args.branch, args.output_path, is_success_required = False)
                    
    elif args.package is not None:
        # verify entry in manifest file
        commit = ""
        version = ""
        found = False
        for i in pkgs:
            if i["name"] == args.package:
                found = True
                commit = i["commit"]
                version = i["version"]
        if not found:
            print(f"Error: package {args.package} is not found in {args.input_manifest}")
            exit(21)
        if args.branch is not None:
            commit = args.branch
            version = args.branch
        if args.check_tag:
            checkout_tag((args.package), version, args.output_path)
        else:
            checkout_commit((args.package), commit, args.output_path)
    else:
        print('Error: please specify "-a" or "-b <pkg>" option.')
        exit(22)
