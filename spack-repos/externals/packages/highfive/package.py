# Copyright 2013-2022 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Highfive(CMakePackage):
    """HighFive - Header only C++ HDF5 interface"""

    homepage = "https://github.com/BlueBrain/HighFive"
    url      = "https://github.com/BlueBrain/HighFive/archive/v1.2.tar.gz"

    version('2.3.1', sha256='41728a1204bdfcdcef8cbc3ddffe5d744c5331434ce3dcef35614b831234fcd7')
    version('2.3',   sha256='7da6815646eb4294f210cec6be24c9234d7d6ceb2bf92a01129fbba6583c5349')
    version('2.2.2', sha256='5bfb356705c6feb9d46a0507573028b289083ec4b4607a6f36187cb916f085a7')
    version('2.2.1', sha256='964c722ba916259209083564405ef9ce073b15e9412955fef9281576ea9c5b85')
    version('2.2',   sha256='fe065f2443e38444100b43999a96916e81a0aa7e500cf768d3bf6f8392b8efee')
    version('2.0',   sha256='deee33d7f578e33dccb5d04771f4e01b89a980dd9a3ff449dd79156901ee8d25')
    version('1.5',   sha256='f194bda482ab15efa7c577ecc4fb7ee519f6d4bf83470acdb3fb455c8accb407')
    version('1.2',   sha256='4d8f84ee1002e8fd6269b62c21d6232aea3d56ce4171609e39eb0171589aab31')
    version('1.1',   sha256='430fc312fc1961605ffadbfad82b9753a5e59482e9fbc64425fb2c184123d395')
    version('1.0',   sha256='d867fe73d00817f686d286f3c69a23731c962c3e2496ca1657ea7302cd0bb944')

    variant('boost', default=False, description='Support Boost')
    variant('mpi', default=True, description='Support MPI')

    depends_on('boost @1.41:', when='+boost')
    depends_on('hdf5 ~mpi', when='~mpi')
    depends_on('hdf5 +mpi', when='+mpi')

    def cmake_args(self):
        args = [
            '-DUSE_BOOST:Bool={0}'.format('+boost' in self.spec),
            '-DHIGHFIVE_PARALLEL_HDF5:Bool={0}'.format('+mpi' in self.spec),
            '-DHIGHFIVE_UNIT_TESTS:Bool=false',
            '-DHIGHFIVE_EXAMPLES:Bool=false']
        return args
