#!/usr/bin/env python3

import os
import yaml
import argparse
import subprocess

import tempfile
import shutil


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


def checkout_ref_and_tag(repo, ref, new_tag):
    outdir = tempfile.mkdtemp()
    cmd = f"""cd {outdir}; \
        git clone https://github.com/DUNE-DAQ/{repo}.git; \
        cd {repo}; \
        git checkout {ref}
    """
    check_output(cmd)
    print(f"Info: checked out {repo:<20} {ref:<20} under {outdir}.")
    cmd = f"""cd {outdir}; \
        cd {repo}; \
        if git ls-remote --exit-code --tags origin {new_tag}; then \
            echo "INFO: {new_tag} exists, deleting it before recreating."; \
            git tag -d {new_tag}; \
            git push --delete origin {new_tag}; \
        fi; \
        git tag -a {new_tag} -m "creating new tag {new_tag}" ; \
        git push origin {new_tag}

    """
    print(check_output(cmd)[0].decode('utf-8'))

    print(f"Info: created {new_tag} for {repo:<20} using {ref:<20}.")
    shutil.rmtree(outdir)
    return


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='checkout-daq-package.py',
        description="Tool for checking out DAQ package(s).",
        epilog="Questions and comments to dingpf@fnal.gov",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-p', '--package', default=None,
                        help='''DAQ package to checkout;''')
    parser.add_argument('-r', '--ref', default=None,
                        help='''git ref, branch tag or commit hash; only to be
                        used with -p option for single package checkout''')
    parser.add_argument('-t', '--tag', default=None, required=True,
                        help='''new tag to be create'''),
    parser.add_argument('-i', '--input-manifest', required=True,
                        help="path to the release manifest file;")
    parser.add_argument('-a', '--all-packages', action='store_true',
                        help="whether to checkout all DAQ pacakges;")
    parser.add_argument('-o', '--output-path', default="./sourcecode",
                        help="path to the output directory;")

    args = parser.parse_args()

    pkgs = []
    if args.input_manifest is not None:
        fman = parse_yaml_file(args.input_manifest)
        pkgs = fman["dunedaq"]
        for i in fman["pymodules"]:
            if i["source"] == "github_DUNE-DAQ":
                pkgs.append(i)
    if len(pkgs) == 0:
        print("Error: proper release manifest file is required.")
        exit(20)

    new_tag = args.tag

    if args.all_packages:
        for i in pkgs:
            iname = i["name"]
            iversion = i["version"]
            if not iversion.startswith('v'):
                iversion = "v" + iversion
            if "elisa" in iname:
                iname.replace('-', '_')
            checkout_ref_and_tag(iname, iversion, new_tag)
    elif args.package is not None:
        # verify entry in manifest file
        ref = ""
        found = False
        for i in pkgs:
            if i["name"] == args.package:
                found = True
                ref = i["version"]
                print(i)
        if not found:
            print(f"Error: package {args.package} is not found in {args.input_manifest}")
            exit(21)
        if args.ref is not None:
            ref = args.ref
        checkout_ref_and_tag((args.package), ref, new_tag)
    else:
        print('Error: please specify "-a" or "-b <pkg>" option.')
        exit(22)
