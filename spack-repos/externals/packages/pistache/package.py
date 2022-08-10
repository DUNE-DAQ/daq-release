# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *
import os
import sys


class Pistache(MesonPackage):
    """An elegant C++ REST framework."""

    homepage = "http://pistache.io"
    url      = "https://github.com/oktal/pistache/archive/v0.0.0.tar.gz"
    git      = "https://github.com/oktal/pistache.git"

    maintainers = ['jcfreeman2']

    # JCF, Oct-23-2021

    # Commit a54a4fab00252a9 dates from Oct-6-2020 on the master
    # branch of pistache, whereas the version in ups is dated
    # Oct-7-2020. Between that and the header files (at the very
    # least) being identical between the ups product and the commit hash's, 
    # it's probably a safe bet that this is the hash we want

    version('dunedaq-v3.2.0', commit="d50d1c293d06744ddff942698738bbc19945fdf5")
    depends_on('openssl')
    depends_on('libpthread-stubs')
    depends_on('meson', type='build')
    depends_on('ninja', type='build')
    depends_on('curl')

    def meson_args(self):
        return ['-DPISTACHE_USE_SSL=true',
                '-DPISTACHE_BUILD_EXAMPLES=false',
                '-DPISTACHE_BUILD_TESTS=false',
                '-DPISTACHE_BUILD_DOCS=false']

    def install(self, spec, prefix):
        super().install(spec, prefix)
        os.system(f"cp -p {self.build_directory}/src/*.so* {self.prefix}/lib64")

    def patch(self):
        os.mkdir(self.prefix + "/lib64")

        copy(join_path(os.path.dirname(__file__),
             "PistacheConfig.cmake"), self.prefix + "/PistacheConfig.cmake")
        copy(join_path(os.path.dirname(__file__),
             "PistacheConfigVersion.cmake"), self.prefix + "/PistacheConfigVersion.cmake")
        copy(join_path(os.path.dirname(__file__),
             "PistacheTargets.cmake"), self.prefix + "/PistacheTargets.cmake")
        copy(join_path(os.path.dirname(__file__),
             "PistacheTargets-release.cmake"), self.prefix + "/PistacheTargets-release.cmake")

