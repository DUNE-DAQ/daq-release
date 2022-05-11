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

    version('22.03', sha256='b2de5f08bcd5733f90d4d7e6c032515908dad8fc8d267ac6a253442d9b83a7c5')
    version('21.11.1', sha256='e0d1c442087ead6759d129ce7d7e3b87b4a01cd71047c621ebc35bb637027658')
    version('21.11', sha256='3246e3ed68ee2b369a5d8be2c06cf108a669e157f4d41c5bcbbb216bf5abd3a1')
    version('21.08', sha256='5352944f65ca6f870df124cca83bf18c2506f7992ece3490346e91654263abb1')
    version('21.05', sha256='1e12499b4c5fcdb57c83e5fe184ea6becddf7cf092893c2c5ef2efb0eec12f0b')
    version('21.02', sha256='09924a2a82551aa29978a3686184f8a105f52f5640b1be287f540b2c91e9489b')
    version('20.11.5', sha256='caad9278dd99386940021355436b20a745daadb6c321893fa5a81d0feb023c7f')
    version('20.11.4', sha256='78028c6a9f4d247b5215ca156b6dbeb03f68a99ca00109c347615a46c1856d6a')
    version('20.11.3', sha256='898680458a4010f421fab760aef47369b74d2954e3560841df3cd7b98076b841')
    version('20.11.2', sha256='cb23efb66e5d9bac79e895c30717828abc9f91938f07fdb0b8785e45cc10487b')
    version('20.11.1', sha256='66c95e81dc4a9b4d1bf5a39d29cb5e353b055065e3a28c579cd13a54c7ba6362')
    version('20.11', sha256='70f76b2a100aa03091fd3daf7073aef21d2c1186bb1e726b857082b997577565')
    version('20.08', sha256='1a33ff04651b5c5ba3da212324bd93bce3e3976fe899d9420ac832d9a459b047')
    version('20.05', sha256='48ca745c0a6c8a109c780a126323121829032c8ce53995844d2ba4c5bfd81d40')
    version('20.02.1', sha256='639a8b66f74a898e1db5c76b999878fcd04875f211bfb1367ece8b9ee9f84438')
    version('20.02', sha256='7893506d35c5f1dc7106247f027da0a25ec307279a3aa8f6931074aab88f4c3a')

    depends_on("py-pyelftools")
    depends_on("libbsd")
