# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Externals(BundlePackage):
    """A dummy package meant to pull in packages needed by DUNE DAQ developers but not developed by them"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    variant("dev", default=True, description="Include build dependencies for a development environment")

    version("XRELEASEX")

    variant('subset', values=('fddaq', 'nddaq', 'fddatautilities'), default='fddaq', description='Select subset of total available external packages')
    
    # Generate from release YAML file
    depends_on("devtools@XRELEASEX", when="+dev")
    # Additional dependencies defined in YAML file to be filled below
