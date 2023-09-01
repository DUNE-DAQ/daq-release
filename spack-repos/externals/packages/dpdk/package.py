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

    version('22.03',   sha256='b2de5f08bcd5733f90d4d7e6c032515908dad8fc8d267ac6a253442d9b83a7c5')
    version('21.11.1', sha256='e0d1c442087ead6759d129ce7d7e3b87b4a01cd71047c621ebc35bb637027658')
    version('21.11',   sha256='3246e3ed68ee2b369a5d8be2c06cf108a669e157f4d41c5bcbbb216bf5abd3a1')
    version('21.08',   sha256='5352944f65ca6f870df124cca83bf18c2506f7992ece3490346e91654263abb1')
    version('21.05',   sha256='1e12499b4c5fcdb57c83e5fe184ea6becddf7cf092893c2c5ef2efb0eec12f0b')
    version('20.02',   sha256='7893506d35c5f1dc7106247f027da0a25ec307279a3aa8f6931074aab88f4c3a')

    depends_on("py-pyelftools")
    depends_on("libbsd")
    depends_on("numactl")
    depends_on("openssl")
    depends_on("libarchive")

    def setup_run_environment(self, env):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_INC", self.prefix + "/include" )
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_LIB", self.prefix + "/lib" )

    def setup_dependent_build_environment(self, env, dependent_spec):
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_INC", self.prefix + "/include" )
        env.set(self.__module__.split(".")[-1].upper().replace("-", "_") + "_LIB", self.prefix + "/lib" )

