#!/usr/bin/env python3

import os
import yaml
import argparse
import shutil
import subprocess
import tempfile
import re
import copy
import tempfile

from time import sleep

from dr_tools import parse_yaml_file
from mappings import cmake_to_spack, pyvenv_url_names

class MyDumper(yaml.Dumper):

    def increase_indent(self, flow=False, indentless=False):
        return super(MyDumper, self).increase_indent(flow, False)

def check_output(cmd, max_tries = 1):

    ntries = 0

    while True:
        ntries += 1

        irun = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
        out = irun.communicate()
        rc = irun.returncode

        if rc == 0:
            break
        elif rc != 0:
            print('\nERROR: command "{}" has exit non-zero exit status,\
    please check!\n'.format(cmd))
            print('Command output:\n {}\n'.format(out[0].decode('utf-8')))
            print('Command error:\n{}\n'.format(out[1].decode('utf-8')))

            if ntries >= max_tries:
                exit(10)
            else:
                sleep_time = 5
                print(f"On try {ntries}; will sleep {sleep_time} seconds before trying again")
                sleep(sleep_time)
    return out


def get_commit_hash(repo, tag_or_branch, fall_back_tag="develop"):
    tmp_dir = tempfile.mkdtemp()

    # Account for packages whose names in release manifest don't match the GitHub URL
    if repo in pyvenv_url_names:
        repo = pyvenv_url_names[repo].get('repo_name', repo)

    try:
        cmd = f"""cd {tmp_dir}; git clone --quiet https://github.com/DUNE-DAQ/{repo}.git"""
        output = check_output(cmd)

        used_ref = tag_or_branch
        repo_dir = os.path.join(tmp_dir, repo)

        is_tag = re.search('\d+.\d+.\d+', tag_or_branch)
        if not is_tag:
            cmd = f"""cd {repo_dir}; \
                if git ls-remote --exit-code --heads origin {tag_or_branch} 2>&1 > /dev/null; then \
                    echo {tag_or_branch}; \
                else \
                    echo {fall_back_tag} ;\
                fi"""
            output = check_output(cmd)
            used_ref = output[0].decode('utf-8').strip()
        else:
            # Python package versions don't start with 'v' in the manifest, but it's needed for checking tags here
            if not is_tag.string.startswith("v"):
                used_ref = f"v{tag_or_branch}"
            cmd = f"""cd {tmp_dir}/{repo}; \
                if ! git show-ref --tags --verify --quiet "refs/tags/{used_ref}"; then \
                    echo "{used_ref} does not exist for package {repo}. Exiting..."; \
                    exit 1; \
                fi;"""
            output = check_output(cmd)
        cmd = f"""cd {repo_dir}; \
            git checkout --quiet {used_ref}; \
            git rev-parse --short HEAD"""
        output = check_output(cmd)
        commit_hash = output[0].decode('utf-8').strip()
        return (used_ref, commit_hash)

    finally:
        shutil.rmtree(tmp_dir)

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

        if not update_hash:
            return

        # Use deepcopy to avoid modifying self.rdict during processing
        pkgs = copy.deepcopy(self.rdict[self.rtype])
        pymodules = [
            copy.deepcopy(pkg)
            for pkg in self.rdict.get("pymodules", [])
            if pkg.get("source") == "github_DUNE-DAQ"
        ]
        all_pkgs = pkgs + pymodules

        for i, pkg in enumerate(all_pkgs):
            iname = pkg.get("name")
            iver  = pkg.get("version")
            ihash = pkg.get("commit")
            itag = iver
            if self.overwrite_branch != "" and (iname != "daq-cmake" or self.overwrite_daq_cmake):
                iver = self.overwrite_branch
            if not iname.startswith('py-'):
                (itag, ihash) = get_commit_hash(iname, iver, pkg.get("version"))

            # Only python modules contain a 'source' field
            if pkg.get("source"):
                for ipy, pymod in enumerate(self.rdict['pymodules']):
                    if pymod.get("name") == iname:
                        self.rdict['pymodules'][ipy]["commit"] = ihash
            else:
                self.rdict[self.rtype][i]["commit"] = ihash
            print(f"Info: {iname:<20} | {itag:<20} | {ihash}")

        # rewrite YAML
        with open(self.yaml, 'w') as outfile:
            outfile.write('---\n')
            yaml.dump(self.rdict, outfile, Dumper=MyDumper, default_flow_style=False, sort_keys=False)
        return

    def get_file_from_package(self, package_name, branch_name, file_name):
        if self.overwrite_branch != '':
            if check_branch_exists(package_name, self.overwrite_branch):
                branch_name = self.overwrite_branch
        file_url = f'https://raw.githubusercontent.com/DUNE-DAQ/{package_name}/{branch_name}/{file_name}'
        command = f'curl -o {file_name} --fail {file_url}'
        check_output(command, 5)

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

    def get_is_db_path_needed(self, package_name, branch_name):
        file_name = "CMakeLists.txt"
        self.get_file_from_package(package_name, branch_name, file_name)

        pattern = re.compile(r"\bdaq_add_dal_library\s*\(")
        with open(file_name, 'r') as infile:
            for line in [l.lstrip() for l in infile]:
                if pattern.search(line) and not line.startswith("#"):
                    return True

        return False

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

                if self.get_is_db_path_needed(ipkg["name"], ipkg["commit"]):
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
        if self.rtype != "coredaq":
            lines += f'\n        depends_on(f"coredaq@{self.rdict["base_release"]} build_type={{build_type}} +dev", when=f"build_type={{build_type}} +dev")'
            lines += f'\n        depends_on(f"coredaq@{self.rdict["base_release"]} build_type={{build_type}} ~dev", when=f"build_type={{build_type}} ~dev")'
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
                    iuser = i["source"].replace("github_", "")
                    # Special cases are handled using a dictionary in mappings.py
                    repo_name = pyvenv_url_names.get(iname, {}).get("repo_name", iname)
                    egg_name = pyvenv_url_names.get(iname, {}).get("egg_name", repo_name)

                    if iversion == "develop" and not iname == "moo":
                        (itag, ihash) = get_commit_hash(iname, iversion, iversion)
                        iline = f"git+https://github.com/{iuser}/{repo_name}@{ihash}#egg={egg_name}"
                    elif iname == "moo":
                        iline = f"git+https://github.com/{iuser}/{repo_name}@{iversion}#egg={egg_name}"
                    else:
                        iline = f"git+https://github.com/{iuser}/{repo_name}@v{iversion}#egg={egg_name}"
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
