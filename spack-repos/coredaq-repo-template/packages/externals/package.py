# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Externals(BundlePackage):
    """A dummy package meant to pull in packages needed by DUNE DAQ developers but not developed by them"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("XRELEASEX")

    variant('subset', values=('fddaq', 'nddaq', 'fddatautilities'), description='Select subset of total available external packages')

    # Generate from release YAML file
    depends_on("devtools@XRELEASEX")
    # Additional dependencies defined in YAML file to be filled below
