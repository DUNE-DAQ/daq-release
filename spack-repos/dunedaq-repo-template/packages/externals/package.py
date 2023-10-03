# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Externals(BundlePackage):
    """A dummy package meant to pull in packages needed by DUNE DAQ developers"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("XRELEASEX")

    # Generate from release YAML file
    depends_on("devtools@XRELEASEX")
    # Additional dependencies defined in YAML file to be filled below
