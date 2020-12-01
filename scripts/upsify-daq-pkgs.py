#!/usr/bin/env python3

import os
import shutil
import tempfile
import subprocess
import getpass
import datetime
import argparse


def check_output(cmd):
    irun = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out = irun.communicate()
    rc = irun.returncode
    if rc != 0:
        print('Error: command "{}" has exit non-zero exit status,\
please check!'.format(cmd))
        print('Output from the commnd: {}'.format(out))
        exit(10)
    return out


def get_version(cdir):
    #cmd = """cd {};git symbolic-ref --short -q HEAD \
    #    || git describe --tags --exact-match 2> /dev/null \
    #    || git rev-parse --short HEAD""".format(dir)
    cmd = "cd {}; git tag --points-at HEAD|tail -n 1 ".format(cdir)
    output = check_output(cmd);
    version = output[0].decode('utf-8').strip()
    return version


def get_commit_hash(cdir):
    cmd = "cd {}; git rev-parse --short HEAD".format(cdir)
    output = check_output(cmd);
    commit_hash = output[0].decode('utf-8').strip()
    return commit_hash


def make_ups_version_file(pkg, ver, chash, equal="e19", dqual="prof"):
    version_content = """FILE = version
PRODUCT = {pkg}
VERSION = {ver}

## Commit Hash: {chash}

#*************************************************
#
FLAVOR = Linux64bit+3.10-2.17
QUALIFIERS = "{equal}:{dqual}"
DECLARER = {user}
DECLARED = {tnow}
MODIFIER = {user}
MODIFIED = {tnow} CDT
PROD_DIR = {pkg}/{ver}
UPS_DIR = ups
TABLE_FILE = {pkg}.table""".format(pkg=pkg, ver=ver, equal=equal, dqual=dqual, user=getpass.getuser(),
                                   chash= chash, tnow=datetime.datetime.now().strftime("%Y-%m-%d %H.%M.%S"))
    return version_content


def make_ups_table_file(pkg, ver, equal="e19", dqual="prof", has_inc=True,
                        has_lib=True, has_bin=False, renamed=False):
    print("making ups table file for {} has_lib {} has_bin {} has_inc {}".format(pkg, has_lib, has_bin, has_inc))
    table_content="""File    = table
Product = {pkg}

#*************************************************
# Starting Group definition
Group:


Flavor     = ANY
Qualifiers = "{equal}:{dqual}"


  Action = GetFQDir
    envSet( ${{UPS_PROD_NAME_UC}}_FQ_DIR, ${{${{UPS_PROD_NAME_UC}}_DIR}}/slf7.x86_64.{equal}.{dqual} )
    fileTest( ${{${{UPS_PROD_NAME_UC}}_FQ_DIR}}, -d, "${{${{UPS_PROD_NAME_UC}}_FQ_DIR}} directory not found: SETUP ABORTED")

  #Action = GetProducts
  #  setupRequired( dunedaq v1_0_0 )


Common:
  Action = setup
    prodDir()
    setupEnv()
    envSet(${{UPS_PROD_NAME_UC}}_VERSION, {ver})
    exeActionRequired(GetFQDir)

    #exeActionRequired(GetProducts)
""".format(pkg=pkg, equal=equal, dqual=dqual, ver=ver)
    if renamed:
        pkg = pkg.replace('_','-')
    if has_lib:
        table_content +="""
    # lib dir
    envSet(${{UPS_PROD_NAME_UC}}_LIB, ${{${{UPS_PROD_NAME_UC}}_FQ_DIR}}/{pkg}/lib64)
    pathPrepend(LD_LIBRARY_PATH, ${{${{UPS_PROD_NAME_UC}}_LIB}})
    pathPrepend(CET_PLUGIN_PATH, ${{${{UPS_PROD_NAME_UC}}_LIB}})
    pathPrepend(CMAKE_PREFIX_PATH, ${{${{UPS_PROD_NAME_UC}}_FQ_DIR}})
    pathPrepend(PKG_CONFIG_PATH, ${{${{UPS_PROD_NAME_UC}}_FQ_DIR}})
""".format(pkg=pkg)
    if has_inc:
        table_content +="""
    # include dir
    envSet(${{UPS_PROD_NAME_UC}}_INC, ${{UPS_PROD_DIR}}/{pkg}/include)
""".format(pkg=pkg)

    if has_bin:
        table_content +="""
    # add the bin directory to the path
    pathPrepend(PATH, ${{${{UPS_PROD_NAME_UC}}_FQ_DIR}}/{pkg}/bin )
    # dropit -E removes non-existent directories
    Execute(dropit -E, NO_UPS_ENV, PATH)
""".format(pkg=pkg)
    table_content += """
End:
# End Group definition
#*************************************************
    """
    return table_content


def create_ups_pkg(install_dir, source_dir, equal, dqual, dest_dir, pkg_name=""):
    flavor = "slf7.x86_64.{}.{}".format(equal, dqual)
    if pkg_name == "":
        pkg_list = [f.name for f in os.scandir(install_dir) if f.is_dir()]
        for ipkg in pkg_list:
            create_ups_pkg(install_dir, source_dir, equal, dqual, dest_dir, ipkg)
        return
    else:
        install_dir = os.path.join(install_dir, pkg_name)
        source_dir = os.path.join(source_dir, pkg_name)
    if not os.path.exists(install_dir):
        print("Error: install directory {} does not exist!".format(install_dir))
        exit(11)
    if  not os.path.exists(source_dir):
        print("Error: sourcecode directoy {} does not exist!".format(source_dir))
        exit(12)
    if not os.path.exists(dest_dir):
        print("Warning: output tarball directory {} does not exist, creating now...".format(dest_dir))
        os.makedirs(dest_dir)
    vdot_ver = get_version(source_dir)
    dot_ver = vdot_ver
    if '.' in vdot_ver:
        version = vdot_ver.replace('.','_')
        if dot_ver.startswith('v'):
            dot_ver = dot_ver[1:]
    else:
        version = vdot_ver # branch name, or commit hash
    commit_hash = get_commit_hash(source_dir)
    tmp_dir = tempfile.mkdtemp()
    renamed = False
    orig_pkg_name = pkg_name
    if '-' in pkg_name:
        pkg_name = pkg_name.replace('-','_')
        renamed = True
    ups_dir = os.path.join(tmp_dir, pkg_name)
    flavor_dir = os.path.join(ups_dir, version, flavor)
    flavor_install_dir = os.path.join(ups_dir, version, flavor, orig_pkg_name)
    print("Info -- creating UPS package: ", pkg_name)
    print("Info -- version: ", version)
    print("Info -- commit hash: ", commit_hash)
    print("Info -- flavor: ", flavor)

    # prepare flavor subdir
    os.makedirs(flavor_dir)
    os.makedirs(flavor_install_dir)
    #shutil.copytree(install_dir, dest)
    copy_install_dir_cmd = "cp -pr {}/* {}".format(install_dir, flavor_install_dir)
    check_output(copy_install_dir_cmd)

    # prepare ups table file
    ups_table_dir = os.path.join(ups_dir, version, "ups")
    os.makedirs(ups_table_dir)
    has_include = False
    has_bin = False
    has_lib = False
    print("tesing include dir {}".format(os.path.exists(os.path.join(install_dir, "include"))))
    if os.path.exists(os.path.join(install_dir, "include")):
        has_include = True
    else:
        has_include = False
    print("tesing lib dir {}".format(os.path.exists(os.path.join(install_dir, "lib64"))))
    if os.path.exists(os.path.join(install_dir, "lib64")):
        has_lib = True
    else:
        has_lib = False
    print("tesing bin dir {}".format(os.path.exists(os.path.join(install_dir, "bin"))))
    if os.path.exists(os.path.join(install_dir, "bin")):
        has_bin = True
    else:
        has_bin = False
    with open("{}/{}.table".format(ups_table_dir, pkg_name), 'w') as tf:
        tcontent = make_ups_table_file(pkg_name, version, equal, dqual, has_lib, has_lib, has_bin, renamed)
        tf.write(tcontent)

    # preprae version dir and version file
    ups_version_dir = os.path.join(ups_dir, "{}.version".format(version))
    os.makedirs(ups_version_dir)
    with open("{}/Linux64bit+3.10-2.17_{}_{}".format(ups_version_dir, equal, dqual), 'w') as vf:
        vcontent = make_ups_version_file(pkg_name, version, commit_hash, equal, dqual)
        vf.write(vcontent)

    # tar up product dir.
    tar_name = "{}-{}-sl7-x86_64-{}-{}.tar.bz2".format(pkg_name, dot_ver, equal, dqual)
    print(tar_name)
    tar_cmd = "cd {} && tar -cvjSf {} {} && mv {} {}".format(
        tmp_dir, tar_name, pkg_name, tar_name, dest_dir)
    print(tar_cmd)
    check_output(tar_cmd)
    #shutil.rmtree(tmp_dir)
    return




# ##MAIN FUNCTION#########

if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        prog='upsify_daq_pkgs.py',
        description="Make UPS products for DAQ packages installed under working directory.",
        epilog="Questions and comments to dingpf@fnal.gov",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-w', '--work-dir', default=None, required=True,
                        help="Path to DAQ software working directory;")
    parser.add_argument('-t', '--tarball-dir', default=None, required=True,
                        help="Path to the destination tarball directory;")
    parser.add_argument('-p', '--package-name', default=None,
                        help="Name of package to be 'UPSified'; if not supplied, all currently installed packages will be 'UPSified';")
    parser.add_argument('-e', '--equalifier', default='e19',
                        help="e qualifier;")
    parser.add_argument('-d', '--debug', action='store_true',
                        help="flag for the 'debug' qualifer ('prof' if unset).")

    args = parser.parse_args()
    workdir = args.work_dir
    install_dir = os.path.join(workdir, "install")
    source_dir = os.path.join(workdir, "sourcecode")
    dest_dir = args.tarball_dir
    equal = args.equalifier
    if args.debug:
        dqual = "debug"
    else:
        dqual = "prof"

    if args.package_name != None:
        create_ups_pkg(install_dir, source_dir, equal, dqual, dest_dir, args.package_name)
    else:
        create_ups_pkg(install_dir, source_dir, equal, dqual, dest_dir)
