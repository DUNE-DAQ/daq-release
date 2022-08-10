# Copyright 2013-2022 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Pugixml(CMakePackage):
    """Light-weight, simple, and fast XML parser for C++ with XPath support"""

    homepage = "https://pugixml.org/"
    url      = "https://github.com/zeux/pugixml/releases/download/v1.10/pugixml-1.10.tar.gz"

    version('1.12.1', sha256='dcf671a919cc4051210f08ffd3edf9e4247f79ad583c61577a13ee93af33afc7')
    version('1.12',   sha256='fd6922a4448ec2f3eb9db415d10a49660e5d84ce20ce66b8a07e72ffc84270a7')
    version('1.11.4', sha256='8ddf57b65fb860416979a3f0640c2ad45ddddbbafa82508ef0a0af3ce7061716')
    version('1.11.3', sha256='aa2a4b8a8907c01c914da06f3a8630d838275c75d1d5ea03ab48307fd1913a6d')
    version('1.11.2', sha256='599eabdf8976aad86ac092a198920d8c127623d1376842bc6d683b12a37fb74f')
    version('1.11.1', sha256='9dce9f0a3756c5ab84ab7466c99972d030021d81d674f5d38b9e30e9a3ec4922')
    version('1.11',   sha256='26913d3e63b9c07431401cf826df17ed832a20d19333d043991e611d23beaa2c')
    version('1.10',   sha256='55f399fbb470942410d348584dc953bcaec926415d3462f471ef350f29b5870a')
    version('1.9',    sha256='d156d35b83f680e40fd6412c4455fdd03544339779134617b9b28d19e11fdba6')
    version('1.8.1',  sha256='00d974a1308e85ca0677a981adc1b2855cb060923181053fb0abf4e2f37b8f39')


    variant('pic', default=True, description='Build position-independent code')
    variant('shared', default=True, description='Build shared libraries')

    conflicts('+shared', when='~pic')

    def cmake_args(self):
        return [
            self.define('BUILD_SHARED_AND_STATIC_LIBS', False),
            self.define_from_variant('BUILD_SHARED_LIBS', 'shared'),
            self.define_from_variant('CMAKE_POSITION_INDEPENDENT_CODE', 'pic'),
        ]
