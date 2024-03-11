#!/usr/bin/env python3

import os
import yaml
import argparse
import shutil
import subprocess
import tempfile
import re

from dr_tools import parse_yaml_file
from mappings import cmake_to_spack

class MyDumper(yaml.Dumper):

    def increase_indent(self, flow=False, indentless=False):
        return super(MyDumper, self).increase_indent(flow, False)

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


def get_commit_hash(repo, tag_or_branch, fall_back_tag="develop"):
    tmp_dir = tempfile.mkdtemp()
    cmd = f"""cd {tmp_dir}; git clone --quiet https://github.com/DUNE-DAQ/{repo}.git"""
    output = check_output(cmd)
    if not tag_or_branch.startswith('v'):
        cmd = f"""cd {tmp_dir}/{repo}; \
            if git ls-remote --exit-code --heads origin {tag_or_branch} 2>&1 > /dev/null; then \
              echo {tag_or_branch}; \
            else \
              echo {fall_back_tag} ;\
            fi"""
        output = check_output(cmd)
        tag_or_branch = output[0].decode('utf-8').strip()
    cmd = f"""cd {tmp_dir}/{repo}; \
        git checkout --quiet {tag_or_branch}; \
        git rev-parse --short HEAD"""
    output = check_output(cmd)
    shutil.rmtree(tmp_dir)
    commit_hash = output[0].decode('utf-8').strip()
    cmd = "cd /tmp; rm -rf daq_repo_*"
    output = check_output(cmd)
    return (tag_or_branch, commit_hash)

def check_branch_exists(repo, branch):
    command = f'git ls-remote --exit-code https://github.com/DUNE-DAQ/{repo}.git --heads origin {branch}'
    args = command.split()
    subproc = subprocess.run(args)
    if subproc.returncode == 0:
        return True
    print(f'WARNING: No branch {branch} exists for package {repo}; defaulting to develop')
    return False

class DAQRelease:

    def __init__(self, yaml_file, overwrite_branch = "", overwrite_daq_cmake = False):
        self.yaml = yaml_file
        self.rdict = parse_yaml_file(self.yaml)
        self.overwrite_branch = overwrite_branch
        self.overwrite_daq_cmake = overwrite_daq_cmake
        self.rtype = self.rdict["type"]

    def set_release(self, release_name, base_release=""):
        if base_release != "":
                self.rdict["base_release"] = base_release
        self.rdict["release"] = release_name

    def copy_release_yaml(self, repo_path, update_hash=False):
        repo_dir = os.path.join(repo_path, "spack-repo")
        os.makedirs(repo_dir, exist_ok=True)
        self.yaml = shutil.copy2(self.yaml, os.path.join(repo_dir, self.rdict["release"] + ".yaml"))
        # Now modify self.yaml and update self.rdict
        pkgs = self.rdict[self.rtype]
        if update_hash:
            for i in range(len(pkgs)):
                ipkg = pkgs[i]
                iname = ipkg["name"]

                if self.overwrite_branch != "" and (iname != "daq-cmake" or self.overwrite_daq_cmake):
                    iver = self.overwrite_branch
                else:
                    iver = ipkg["version"]
                ihash = ipkg["commit"]
                itag = iver
                if not iname.startswith('py-'):
                    (itag, ihash) = get_commit_hash(iname, iver, ipkg["version"])
                self.rdict[self.rtype][i]["commit"] = ihash
                print(f"Info: {iname:<20} | {itag:<20} | {ihash}")

            # rewrite YAML
            with open(self.yaml, 'w') as outfile:
                outfile.write('---\n')
                yaml.dump(self.rdict, outfile, Dumper=MyDumper, default_flow_style=False, sort_keys=False)
        return

    def get_cmake_dependencies(self, package_name, branch_name='develop'):
        if self.overwrite_branch != '':
            if check_branch_exists(package_name, self.overwrite_branch):
                branch = self.overwrite_branch
        file_name = "CMakeLists.txt"
        cmakelists_path = f'https://raw.githubusercontent.com/DUNE-DAQ/{package_name}/{branch_name}/{file_name}'
        command = f'curl -o {file_name} --fail {cmakelists_path}'
        check_output(command)
        args = command.split()
        subprocess.run(args)

        cmake_dependencies_list = []
        with open(file_name, 'r') as infile:
            lines = infile.read()
            # Parse package names from find_package calls. Everything up to the first
            # white space character will be taken as the package name (i.e., no "REQUIRED"
            # or "COMPONENTS"
            find_package_pattern  = re.compile(r'[^# ]find_package\(\s*([^)\s]+)')
            cmake_dependencies_list = find_package_pattern.findall(lines)
            # Special cases where the dependency has no explicit find_package call
            find_daq_codegen = re.search("daq_codegen\(", lines)
            if find_daq_codegen:
                cmake_dependencies_list.append('py-moo')
            find_numa = re.search("pkg_check_modules\(numa", lines)
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
                # Frozen release
                else:
                    lines = lines.replace("XVERSIONX", ipkg["version"])
                if ipkg["commit"] is not None:
                    lines = lines.replace("XHASHX", ipkg["commit"])
                # Infer dependencies from CMakeLists.txt
                cmake_package_list = self.get_cmake_dependencies(ipkg["name"])
                depends_on_list = self.generate_depends_on_list(cmake_package_list)
                lines = lines.replace("XDEPENDSX", depends_on_list)
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

            # now add additional deps:
            for idep in self.rdict[ipkg]:
                iname = idep["name"]
                iver = idep["version"]
                # Externals, system/devtools etc, variant is used instead of
                # version
                ivar = idep["variant"]
                if ivar == None:
                    lines += f'\n    depends_on("{iname}@{iver}")'
                else:
                    lines += f'\n    depends_on("{iname}@{iver} {ivar}")'
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

        # now add additional deps:
        lines += '\n    for build_type in ["Debug", "RelWithDebInfo", "Release"]:'
        if self.rtype != "dunedaq":
            lines += f'\n        depends_on(f"dunedaq@{self.rdict["base_release"]} build_type={{build_type}}", when=f"build_type={{build_type}}")'
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
        if self.rtype == "dunedaq":
            self.generate_external_umbrella_package(repo_path, template_dir)
        self.generate_daq_umbrella_package(repo_path, template_dir)
        return

    def generate_repo(self, outdir, tempdir, update_hash, release_name, base_release):
        if release_name is not None:
            self.set_release(release_name, base_release)
        self.copy_release_yaml(outdir, update_hash)
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
                    iline = f"git+https://github.com/"
                    iuser = i["source"].replace("github_", "")
                    if iname == "moo":
                        iline = f"git+https://github.com/{iuser}/{iname}@{iversion}#egg={iname}"
                    elif iname == "elisa-client-api":
                        iline = f"git+https://github.com/{iuser}/elisa_client_api@v{iversion}#egg={iname}"
                    elif iname == "connectivityserver":
                        iline = f"git+https://github.com/{iuser}/{iname}@v{iversion}#egg=connection-service"
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
    parser.add_argument('--base-release',
                        help="base release name")

    args = parser.parse_args()

    daq_release = DAQRelease(args.input_manifest, args.overwrite_branch, args.overwrite_daq_cmake)
    if args.pypi_manifest:
        os.makedirs(args.output_path, exist_ok=True)
        outfile = os.path.join(args.output_path, 'pypi_manifest.sh')
        daq_release.generate_pypi_manifest(outfile)
    elif args.pyvenv_requirements:
        os.makedirs(args.output_path, exist_ok=True)
        outfile = os.path.join(args.output_path, 'pyvenv_requirements.txt')
        daq_release.generate_pyvenv_requirements(outfile)
    elif args.check_branch:
        tmp_dir = tempfile.mkdtemp()
        daq_release.copy_release_yaml(tmp_dir, True)
        shutil.rmtree(tmp_dir)
    else:
        daq_release.generate_repo(args.output_path, args.template_path,
                                  args.update_hash, args.release_name,
                                  args.base_release)
