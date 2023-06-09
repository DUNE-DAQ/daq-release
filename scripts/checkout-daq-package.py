#!/usr/bin/env python3

import os
import yaml
import argparse
import subprocess


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


def check_output(cmd):
    irun = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out = irun.communicate()
    rc = irun.returncode
    if rc != 0:
        print('\nERROR: command "{}" has exit non-zero exit status,\
please check!\n'.format(cmd))
        print('Command output:\n {}\n'.format(out[0].decode('utf-8')))
        print('Command error:\n{}\n'.format(out[1].decode('utf-8')))

        exit(10)
    return out


def checkout_commit(repo, commit, outdir):
    cmd = f"""mkdir -p {outdir}; cd {outdir}; \
        git clone https://github.com/DUNE-DAQ/{repo}.git; \
        cd {repo}; \
        git checkout {commit}
    """
    check_output(cmd)
    print(f"Info: checked out {repo:<20} {commit:<20} under {outdir}.")
    return

def checkout_tag(repo, commit, outdir):
    cmd = f"""mkdir -p {outdir}; cd {outdir}; \
        git clone https://github.com/DUNE-DAQ/{repo}.git; \
        cd {repo}; \
        git checkout {commit}; \
        cmake_version=`grep "^project" CMakeLists.txt |grep ")$"|grep -oP "(([[:digit:]]+\.)([[:digit:]]+\.)([[:digit:]]+))"`; \
        tag=v"$cmake_version"; \
        echo $tag ;\
        echo $commit; \
        if [[ $tag != "{commit}" ]]; then echo "Tag mismatches version in CMakeLists.txt" && exit 1; fi
    """
    check_output(cmd)
    print(f"Info: verified version in CMake, checked out {repo:<20} {commit:<20} under {outdir}.")
    return


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='checkout-daq-package.py',
        description="Tool for checking out DAQ package(s).",
        epilog="Questions and comments to dingpf@fnal.gov",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-p', '--package', default=None,
                        help='''DAQ package to checkout;''')
    parser.add_argument('-b', '--branch', default=None,
                        help='''branch/tag name, or commit hash; only to be
                        used with -p option for single package checkout''')
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
        pkgs = parse_yaml_file(args.input_manifest)["dunedaq"]
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
                checkout_commit(i["name"], i["commit"], args.output_path)
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
