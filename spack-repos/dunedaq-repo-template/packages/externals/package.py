# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Externals(BundlePackage):
    """A dummy package meant to pull in packages needed by DUNE DAQ developers"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("XRELEASEX")

    variant('dev', default=False, description='Load in tools used to develop DUNE DAQ packages')

    # Generate from release YAML file
    depends_on("devtools@XRELEASEX", when="+dev")
    depends_on("systems@XRELEASEX", when="~dev")
    # Additional dependencies defined in YAML file to be filled below
