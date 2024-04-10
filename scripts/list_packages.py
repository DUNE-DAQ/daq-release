#!/usr/bin/env python3

import os
import sys

from spack.dr_tools import get_packages

if len(sys.argv) < 3:
    print("Usage: {} <name of release type (develop or version)> <name of build group (coredaq, fddaq, nddaq>".format(sys.argv[0].split("/")[-1]))
    sys.exit(1)

reltype=sys.argv[1]
bg=sys.argv[2]

thisdir=os.path.dirname( os.path.realpath(__file__) )
yaml_filename="{}/../configs/{}/{}-{}/release.yaml".format(thisdir, bg, bg, reltype)

if not os.path.exists(yaml_filename):
    print(f"Unable to find {yaml_filename}; perhaps directories in daq-release were rearranged since this script was written?")
    sys.exit(2)

print(" ".join( get_packages(yaml_filename) ))


