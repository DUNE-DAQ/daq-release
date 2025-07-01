#!/usr/bin/env python3
import argparse
import os
import sys

from spack.dr_tools import get_packages
PACKAGE_GROUPS = ["coredaq", "fddaq", "nddaq", "externals"]

parser = argparse.ArgumentParser(description="List the DUNE-DAQ repositories")
parser.add_argument("--release-type", help="name of release type (\"develop\", \"production_v4\", or stable release version as vX.Y.Z)", default="develop")
parser.add_argument("--package-group", choices=PACKAGE_GROUPS, default="coredaq", help="Package group to check. Default is 'coredaq'.", required=True)
parser.add_argument("--py-modules", help="only do python repository, else do only c++", action="store_true")
args = parser.parse_args()
pg = args.package_group
reltype = args.release_type
py = args.py_modules

subdir = pg if pg != "externals" else "coredaq"

thisdir=os.path.dirname( os.path.realpath(__file__) )
yaml_filename="{}/../configs/{}/{}-{}/release.yaml".format(thisdir, subdir, subdir, reltype)

if not os.path.exists(yaml_filename):
    print(f"Unable to find {yaml_filename}; perhaps directories in daq-release were rearranged since this script was written?")
    sys.exit(2)

if py:
    pg = "pymodules"
print(" ".join( get_packages(yaml_filename, pg) ))


