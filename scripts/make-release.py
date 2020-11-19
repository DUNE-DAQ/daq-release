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
        print("Error: Field {} does not exist in the manifest file!".format(
            fkey))
    return fvalue


def append_to_file(fname, out_string):
    with open(fname, 'a') as ofile:
        ofile.write(out_string)
    return


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


def cmd_setup_product_path(fname, fman):
    setup_string = ""
    for i in fman["product_paths"]:
        setup_string += ". {}/setup\n".format(i)
        setup_string += """if [[ "$?" != 0 ]]; then
  echo "Executing \". {}/setup\" returned a nonzero value; returning..."
  return 10
fi\n""".format(i)

    append_to_file(fname, setup_string)
    return setup_string


def cmd_products_setup(fname, fman, fsection):
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
  echo "All setups of products completed successfully.s"
else
  echo "One or more of setups failed; returning..." >&2
  return 1
fi
"""
    append_to_file(fname, setup_string)
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
    parser.add_argument('-t', '--tarball', action='store_true',
                        help='''Generate bash commands for \
                        setting up UPS product path;''')
    parser.add_argument('-i', '--input-manifest',
                        default=None,
                        help="path to the release manifest file;")

    args = parser.parse_args()

    fman = parse_release_file(args.input_manifest)

    print(yaml.dump(fman, default_flow_style=False, sort_keys=False))

    release = get_field(fman, "release")
    if release != "develop":
        release = release.replace('.', '-')

    print("INFO: release is: ... {}".format(release))
