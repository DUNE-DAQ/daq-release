# Copyright 2013-2019 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *
import sys
import os
#libdir="%s/var/spack/repos/fnal_art/lib" % os.environ["SPACK_ROOT"]
#if not libdir in sys.path:
#    sys.path.append(libdir)



def patcher(x):
    cetmodules_20_migrator(".","artg4tk","9.07.01")


def sanitize_environments(*args):
    for env in args:
        for var in ('PATH', 'CET_PLUGIN_PATH',
                    'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH', 'LIBRARY_PATH',
                    'CMAKE_INSTALL_RPATH', 'CMAKE_PREFIX_PATH', 'ROOT_INCLUDE_PATH'):
            env.prune_duplicate_paths(var)
            env.deprioritize_system_paths(var)


class CetlibExcept(CMakePackage):
    """Exception libraries for the art suite."""

    homepage = 'https://github.com/art-framework-suite/cetlib-except'
    git_base = 'https://github.com/art-framework-suite/cetlib-except.git'
    url = 'https://github.com/art-framework-suite/cetlib-except/archive/refs/tags/v1_07_04.tar.gz'

    version('1.07.04', tag='v1_07_04', git=git_base, get_full_repo=True)


    variant('cxxstd',
            default='17',
            values=('14', '17'),
            multi=False,
            description='Use the specified C++ standard when building.')

    depends_on('cmake', type='build')

    for build_type in ["Debug", "Release", "RelWithDebInfo"]:
        depends_on(f'cetmodules build_type={build_type}', when=f'build_type={build_type}', type='build')
#        depends_on(f'cetpkgsupport build_type={build_type}', when=f'build_type={build_type}', type=('build','run'))
    
    depends_on('catch2', type=('build','run'))

    if 'SPACKDEV_GENERATOR' in os.environ:
        generator = os.environ['SPACKDEV_GENERATOR']
        if generator.endswith('Ninja'):
            depends_on('ninja', type='build')

#    def url_for_version(self, version):
#        url = 'https://cdcvs.fnal.gov/cgi-bin/git_archive.cgi/cvs/projects/cetlib_except.v{0}.tbz2'
#        return url.format(version.underscored)

#    def cmake_args(self):
#        args = ['-DCMAKE_CXX_STANDARD={0}'.
#                format(self.spec.variants['cxxstd'].value)]
#        return args

    def cmake_args(self):
        spec = self.spec
        cmake_args = [
            '-DCMAKE_INSTALL_PREFIX:PATH={0}'.format(spec.prefix),
            '-DCMAKE_CXX_STANDARD={0}'.format(self.spec.variants['cxxstd'].value)
        ]
        return cmake_args

    def setup_environment(self, spack_env, run_env):
        # For tests.
        spack_env.prepend_path('PATH', os.path.join(self.build_directory, 'bin'))
        # Cleanup.
        sanitize_environments(spack_env, run_env)

    @run_after('install')
    def rename_README(self):
        import os
        os.rename( join_path(self.spec.prefix, "README"),
                   join_path(self.spec.prefix, "README_%s"%self.spec.name))
