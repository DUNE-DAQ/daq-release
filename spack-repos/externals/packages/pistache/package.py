# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *
import os
import sys

#class Pistache(MesonPackage):
class Pistache(CMakePackage):
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

    # JCF, Dec-5-2024

    # Commit 6e59eb21b495a7a is just the current head of the master
    # branch; except for an isolated incident in 2022 there are no
    # Pistache tags. The hope is this will build correctly under gcc
    # 13.2, which was not the case with the dunedaq-v2.8.0 version
    # below

    #version('fddaq-v5.3.0', commit="6e59eb21b495a7a")
    version('dunedaq-v2.8.0', commit="a54a4fab00252a9")
    version('master', branch='master')
    depends_on('openssl')
    depends_on('libpthread-stubs')
    depends_on('rapidjson')

    #patch('pistache_gcc12.patch', when='@dunedaq-v2.8.0')
    patch('build_under_gcc_13.2.0.patch', when='@dunedaq-v2.8.0')

    def install(self, spec, prefix):

        super().install(spec, prefix)
        os.makedirs(self.prefix + "/lib64", exist_ok=True)

        os.system(f"cp -p {self.build_directory}/src/*.so* {self.prefix}/lib64")

        copy(join_path(os.path.dirname(__file__),
             "PistacheConfig.cmake"), self.prefix + "/PistacheConfig.cmake")
        copy(join_path(os.path.dirname(__file__),
             "PistacheConfigVersion.cmake"), self.prefix + "/PistacheConfigVersion.cmake")
        copy(join_path(os.path.dirname(__file__),
             "PistacheTargets.cmake"), self.prefix + "/PistacheTargets.cmake")
        copy(join_path(os.path.dirname(__file__),
             "PistacheTargets-release.cmake"), self.prefix + "/PistacheTargets-release.cmake")
        copy(join_path(os.path.dirname(__file__),
             "CMakeLists.txt"), self.stage.source_path + "/CMakeLists.txt")
