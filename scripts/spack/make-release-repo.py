#!/usr/bin/env python3

import os
import yaml
import argparse


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


class DAQRelease:
    def __init__(self, yaml_file):
        self.yaml = yaml_file
        self.rdict = parse_yaml_file(self.yaml)

    def generate_repo_file(self, repo_path):
        repo_dir = os.path.join(repo_path, self.rdict["release"])
        os.makedirs(repo_dir)
        with open(os.path.join(repo_dir, "repo.yaml"), 'w') as f:
            repo_string = "repo:\n  namespace: '{}'\n".format(
                self.rdict["release"])
            f.write(repo_string)
        return

    def generate_daq_package(self, repo_dir, template_dir):
        repo_dir = os.path.join(repo_dir, self.rdict["release"], "packages")
        template_dir = os.path.join(template_dir, "packages")
        for ipkg in self.rdict["dunedaq"]:
            itemp = os.path.join(template_dir, ipkg["name"], 'package.py')
            if not os.path.exists(itemp):
                print(f"Error: template file {itemp} is not found!")
                continue
            with open(itemp, 'r') as f:
                lines = f.read()
                lines = lines.replace("VERSION", ipkg["version"])
                lines = lines.replace("HASH", ipkg["commit"])
                # get commit hash
            ipkg_dir = os.path.join(repo_dir, ipkg["name"])
            os.makedirs(ipkg_dir)
            ipkgpy = os.path.join(ipkg_dir, "package.py")
            with open(ipkgpy, 'w') as o:
                o.write(lines)
                print(f"Info: package.py has been written at {ipkgpy}.")
        return

    def generate_umbrella_package(self, repo_dir, template_dir):
        repo_dir = os.path.join(repo_dir, self.rdict["release"], "packages")
        template_dir = os.path.join(template_dir, "packages")
        for ipkg in ['devtools', 'externals', 'systems', 'dunedaq']:
            itemp = os.path.join(template_dir, ipkg, 'package.py')
            if not os.path.exists(itemp):
                print(f"Error: template file {itemp} is not found!")
                continue
            with open(itemp, 'r') as f:
                lines = f.read()
                lines = lines.replace("RELEASE", self.rdict["release"])

            # now add additional deps:
            if ipkg == 'dunedaq':
                lines += '\n    for build_type in ["Debug", "RelWithDebInfo", "Release"]:'
            for idep in self.rdict[ipkg]:
                iname = idep["name"]
                iver = idep["version"]
                if ipkg != 'dunedaq':
                    ivar = idep["variant"]
                    if ivar == None:
                        lines += f'\n    depends_on("{iname}@{iver}")'
                    else:
                        lines += f'\n    depends_on("{iname}@{iver} {ivar}")'
                else:
                    lines += f'\n        depends_on(f"{iname}@{iver} build_type={{build_type}}", when=f"build_type={{build_type}}")'
            lines += '\n'
            ipkg_dir = os.path.join(repo_dir, ipkg)
            os.makedirs(ipkg_dir)
            ipkgpy = os.path.join(ipkg_dir, "package.py")
            with open(ipkgpy, 'w') as o:
                o.write(lines)
                print(f"Info: package.py has been written at {ipkgpy}.")
        return


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='make-release-repo.py',
        description="Parse DUNE DAQ release manifest files.",
        epilog="Questions and comments to dingpf@fnal.gov",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-t', '--template-path',
                        default="../../spack-repos/packages",
                        help='''path to the template directory;''')
    parser.add_argument('-i', '--input-manifest', required=True,
                        help="path to the release manifest file;")
    parser.add_argument('-o', '--output-path', required=True,
                        help="path for the generated repo;")

    args = parser.parse_args()

    daq_release = DAQRelease(args.input_manifest)
    daq_release.generate_repo_file(args.output_path)
    daq_release.generate_daq_package(args.output_path, args.template_path)
    daq_release.generate_umbrella_package(args.output_path, args.template_path)
