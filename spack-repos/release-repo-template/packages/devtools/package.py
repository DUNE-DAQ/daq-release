# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Devtools(BundlePackage):
    """An umbrella package which pulls in the DUNE DAQ development tools"""

    homepage = "XHOMEPAGEX"

    version("XRELEASEX")

    # Generate from release YAML file
    depends_on("systems@XRELEASEX")
    # Additional dependencies defined in YAML file to be filled below
