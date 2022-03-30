# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Devtools(BundlePackage):
    """The DUNE DAQ development tools"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("RELEASE")

    # Generate from release YAML file
    depends_on("systems@RELEASE")
    # Additional dependencies defined in YAML file to be filled below
