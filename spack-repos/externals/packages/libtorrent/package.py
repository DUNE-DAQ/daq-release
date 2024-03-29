# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class Libtorrent(CMakePackage):

    homepage = "https://www.example.com"
    git = "https://github.com/arvidn/libtorrent.git"


    version("2.0.9", commit="4d0b6c7433f8aa42cfcc54f7923adca1d0015f72", submodules=True)

    depends_on('cmake', type='build')
    generator = 'Ninja'
    depends_on('ninja', type='build')
    depends_on('boost')
    depends_on('openssl')

    def cmake_args(self):
        args = ["-DCMAKE_CXX_STANDARD=17"]
        return args

    def setup_run_environment(self, env):
        env.prepend_path('LIBRARY_PATH', self.spec['openssl'].prefix.lib)
        env.prune_duplicate_paths('LIBRARY_PATH')


    def setup_build_environment(self, env):
        env.prepend_path('LIBRARY_PATH', self.spec['openssl'].prefix.lib)
        env.prune_duplicate_paths('LIBRARY_PATH')

    def setup_dependent_run_environment(self, env, dependent_spec):
        env.prepend_path('LIBRARY_PATH', self.spec['openssl'].prefix.lib)
        env.prune_duplicate_paths('LIBRARY_PATH')


    def setup_dependent_build_environment(self, env, dependent_spec):
        env.prepend_path('LIBRARY_PATH', self.spec['openssl'].prefix.lib)
        env.prune_duplicate_paths('LIBRARY_PATH')
