# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Folly(CMakePackage):
    """Folly (acronymed loosely after Facebook Open Source Library) is a
    library of C++11 components designed with practicality and efficiency
    in mind.

    Folly contains a variety of core library components used extensively at
    Facebook. In particular, it's often a dependency of Facebook's other open
    source C++ efforts and place where those projects can share code.
    """

    homepage = "https://github.com/facebook/folly"
    url = "https://github.com/facebook/folly/releases/download/v2021.05.24.00/folly-v2021.05.24.00.tar.gz"

    version("2025.11.24.00", sha256="46fec61b588637f90a1e4f1da07473ec25f6a9d38f11a92bbc4ae25dcade7cf7")
    version("2024.12.02.00", sha256="c6656ebdcade0f98925754d02a270b5c3b1d5a3a7cf16b468455fe2fc907569c")
    version('2021.12.13.00', sha256='87f87f5c6bf101ef15322c7351039747fb73640504d3d6de1fb719428fb0a5bc')
    version('2021.05.24.00', sha256='9d308adefe4670637f5c7d96309b3b394ac3fa129bc954f5dfbdd8b741c02aad')

    # CMakePackage Dependency
    depends_on('pkgconfig', type='build')

    # folly requires gcc 4.9+ and a version of boost compiled with >= C++14
    # TODO: Specify the boost components
    variant('cxxstd', default='14', values=('14', '17', '2a'), multi=False, description='Use the specified C++ standard when building.')
    depends_on('boost+context+container+filesystem+regex+date_time+system+thread+program_options cxxstd=14', when='cxxstd=14')
    depends_on('boost+context+container+filesystem+regex+date_time+system+thread+program_options cxxstd=17', when='cxxstd=17')
    depends_on('boost+context+container+filesystem+regex+date_time+system+thread+program_options cxxstd=2a', when='cxxstd=2a')

    # required dependencies
    depends_on('gflags')
    depends_on('glog@:0.6', when='@2024.12.02.00:') # Build complaints about glog header inclusion for later versions of glog
    depends_on('glog@:0.4', when='@:2021.12.13.00')                        # For older versions of folly have traditionally used glog 0.4
    depends_on('double-conversion')
    depends_on('libevent')
    depends_on('fmt')
    depends_on('fast-float', when='@2024.12.02.00:')

    # optional dependencies
    variant('libdwarf', default=False, description="Optional Dependency")
    variant('elfutils', default=False, description="Optional Dependency")
    variant('libunwind', default=False, description="Optional Dependency")
    depends_on('libdwarf', when='+libdwarf')
    depends_on('elfutils', when='+elfutils')
    depends_on('libunwind', when='+libunwind')

    # Add Phil's gflags patch from https://github.com/DUNE-DAQ/dunedaq-spack
    patch('find-gflags-shared.diff')

    configure_directory = 'folly'

    # JCF, Dec-30-2024
    # Define FOLLY_F14_FORCE_FALLBACK to avoid folly linking problems 
    # See F14LinkCheck in folly for more on this

    def cmake_args(self):
        return ['-DBUILD_SHARED_LIBS=ON',
                '-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
                '-DCMAKE_CXX_FLAGS=-DFOLLY_F14_FORCE_FALLBACK=1']
