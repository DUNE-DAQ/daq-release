#!/usr/bin/env python3

import os
import yaml
import argparse
import subprocess
import tarfile


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


def get_field(fman, fkey):
    try:
        fvalue = fman[fkey]
    except KeyError:
        print("Field {} does not exist in the manifest file!".format(fkey))
    return fvalue


def parse_release_file(fname):
    if not os.path.exists(fname):
        print("Error: -- Manifest file {} does not exist".format(fname))
        exit(20)
    fman = ""
    with open(fname, 'r') as stream:
        try:
            fman = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
    return fman


def cmd_setup_product_path(fman):
    setup_string = ""
    for i in fman["product_paths"]:
        setup_string += ". {}/setup\n".format(i)
        setup_string += """if [[ "$?" != 0 ]]; then
  echo "Executing \". {}/setup\" resulted in a nonzero return value; returning..."
  return 10
fi\n""".format(i)
    print(setup_string)
    return setup_string


def cmd_products_setup(fman, fsection):
    """return line seperated UPS setup commands to be used in 'eval' in bash
    scripts"""
    setup_string = 'setup_returns=""\n'
    for i in fman[fsection]:
        if i["variant"] is not None:
            setup_string += "setup {} {} -q {}\n".format(
                i["name"], i["version"], i["variant"])
        else:
            setup_string += "setup {} {}\n".format(
                i["name"], i["version"])
        setup_string += 'setup_returns=$setup_returns"$? "\n'
    setup_string += """if ! [[ "$setup_returns" =~ [1-9] ]]; then
  echo "All setup calls on the packages returned 0, indicative of success"
else
  echo "At least one of the required packages this script attempted to set up didn't set up correctly; returning..." >&2
  return 1
fi
"""
    print(setup_string)
    return setup_string


def make_tarball(output_filename, source_dir):
    with tarfile.open(output_filename, "w:gz") as tar:
        tar.add(source_dir, arcname=os.path.basename(source_dir))
    return


def make_bundle_pacakge(pkg_list, bundle_name, bundle_version):
    # create UPS bundle package.
    # Make UPS table file...
    return


# ##MAIN FUNCTION#########

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='make-release.py',
        description="Parse DUNE DAQ release manifest files.",
        epilog="Questions and comments to dingpf@fnal.gov",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--release-tarball', action='store_true',
                        help='''Generate bash commands for \
                        setting up UPS product path;''')
    parser.add_argument('--release-relpath', action='store_true',
                        help='''generate line separated bash \
                        commands of setting up UPS products for \
                        external dependencies;''')
    parser.add_argument('--setup-prebuilt', action='store_true',
                        help='''generate line separated bash commands \
                        of setting up ups products for prebuilt daq \
                        packages;''')
    parser.add_argument('--git-checkout', action='store_true',
                        help='''Run git clone and checkout commands \
                        for DAQ source packages from GitHub repos;''')
    parser.add_argument('-s', '--src-dir', default='./sourcecode',
                        help="source code directory;")
    parser.add_argument('-r', '--release', default='develop',
                        help="set the DAQ release to use;")
    parser.add_argument('-p', '--path-to-manifest', default='./daq-release',
                        help="set the path to DAQ release manifest files;")
    parser.add_argument('-u', '--users-manifest',
                        default=None,
                        help="set the path to user's manifest files;")

    args = parser.parse_args()

    if args.release == "develop":
        release = "develop"
    else:
        release = args.release.replace('.', '-')
    release_manifest = "{}/release_{}.yaml".format(
            args.path_to_manifest, release)
    user_manifest = args.users_manifest

    fnames = [release_manifest]
    if user_manifest is not None:
        fnames.append(user_manifest)

    # print(fman)
    # print(yaml.dump(fman, default_flow_style=False, sort_keys=False))
