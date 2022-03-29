# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Devtools(BundlePackage):
    """The DUNE DAQ development tools"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("develop")
    version("dunedaq-v2.9.0")
    version("dunedaq-v2.8.2")

    for ver in ["develop", "dunedaq-v2.8.2", "dunedaq-v2.9.0"]:

        depends_on("systems@dunedaq-v2.9.0", when=f"@{ver}")
        depends_on("cmake@3.20.5", when=f"@{ver}")  # Should be 3.17.2, but hep-concurrency needs a newer CMake version
        depends_on("gdb@9.2~python", when=f"@{ver}")
        depends_on("ninja@1.10.0", when=f"@{ver}")
