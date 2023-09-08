# Copyright 2013-2022 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Highfive(CMakePackage):
    """HighFive - Header only C++ HDF5 interface"""

    homepage = "https://github.com/BlueBrain/HighFive"
    url      = "https://github.com/BlueBrain/HighFive/archive/v2.7.1.tar.gz"

    version('2.7.1', sha256='25b4c51a94d1e670dc93b9b73f51e79b65d8ff49bcd6e5d5582d5ecd2789a249')
    version('2.4.1', sha256='6826471ef5c645ebf947d29574b302991525a8a8ff1ef687aba7311d9a0ea36f')
    version('2.4.0', sha256='ba0ed6d8e2e09e80849926f38c15a26cf4b80772084cea0555269a25fec02149')
    version('2.3.1', sha256='41728a1204bdfcdcef8cbc3ddffe5d744c5331434ce3dcef35614b831234fcd7')
    version('2.3',   sha256='7da6815646eb4294f210cec6be24c9234d7d6ceb2bf92a01129fbba6583c5349')
    version('2.2.2', sha256='5bfb356705c6feb9d46a0507573028b289083ec4b4607a6f36187cb916f085a7')
    version('2.2.1', sha256='964c722ba916259209083564405ef9ce073b15e9412955fef9281576ea9c5b85')
    version('2.2',   sha256='fe065f2443e38444100b43999a96916e81a0aa7e500cf768d3bf6f8392b8efee')
    version('2.1.1', sha256='52cffeda0d018f020f48e5460c051d5c2031c3a3c82133a21527f186a0c1650e')
    version('2.1',   sha256='cc9e93baecc939c6984f220643338092b7e71ef666cb1e1c80f3dfde0eaa89f2')
    version('1.2',   sha256='4d8f84ee1002e8fd6269b62c21d6232aea3d56ce4171609e39eb0171589aab31')

    patch('cmake_deps.patch', sha256='84200e200e56f90ce91f0bceb67494ebaf2a1f6c07d1c37d886701292b078d57', when='@2.7.1')

    variant('mpi', default=False, description='Support MPI')
    variant('boost', default=False, description='Support Boost')

    depends_on('boost @1.41:', when='+boost')
    depends_on('hdf5 ~mpi +threadsafe', when='~mpi')
    depends_on('hdf5 +mpi +threadsafe', when='+mpi')

    def cmake_args(self):
        print(self.spec)
        args = [
            '-DUSE_BOOST:Bool={0}'.format('+boost' in self.spec),
            '-DHIGHFIVE_PARALLEL_HDF5:Bool={0}'.format('+mpi' in self.spec),
            '-DHIGHFIVE_UNIT_TESTS:Bool=false',
            '-DHIGHFIVE_EXAMPLES:Bool=false']
        return args
