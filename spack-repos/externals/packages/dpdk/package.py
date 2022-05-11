# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Dpdk(MesonPackage):
    """Data Plane Development Kit: accelerate packet processing workloads"""

    homepage = "https://www.dpdk.org"
    url      = "https://fast.dpdk.org/rel/dpdk-21.11.tar.xz"

    maintainers = ['jcfreeman2']

    version('21.11', sha256='3246e3ed68ee2b369a5d8be2c06cf108a669e157f4d41c5bcbbb216bf5abd3a1')

    depends_on("py-pyelftools")
    depends_on("libbsd")
