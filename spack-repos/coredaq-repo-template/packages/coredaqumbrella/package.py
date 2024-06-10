# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Coredaqumbrella(BundlePackage):
    """A dummy package to ensure that dbe uses the same packages as coredaq"""

    homepage = "https://dune-daq-sw.readthedocs.io/en/latest/"

    version("uninstallme")

    depends_on("coredaq")
    depends_on("dbe")
