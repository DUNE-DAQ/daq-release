# Copyright 2013-2019 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *
import sys
from spack.environment import *
import os
import os.path

#libdir="%s/var/spack/repos/fnal_art/lib" % os.environ["SPACK_ROOT"]
#if not libdir in sys.path:
#    sys.path.append(libdir)



def patcher(x):
    os.system("patch -p1 < %s/p1" % os.path.dirname(__file__))
    cetmodules_20_migrator(".","artg4tk","9.07.01")

def sanitize_environments(*args):
    for env in args:
        for var in ('PATH', 'CET_PLUGIN_PATH',
                    'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH', 'LIBRARY_PATH',
                    'CMAKE_INSTALL_RPATH', 'CMAKE_PREFIX_PATH', 'ROOT_INCLUDE_PATH'):
            env.prune_duplicate_paths(var)
            env.deprioritize_system_paths(var)


class Cetlib(CMakePackage):
    """A utility library for the art suite."""

    homepage = 'https://github.com/art-framework-suite/cetlib'
    git_base = 'https://github.com/art-framework-suite/cetlib.git'
    url = 'https://github.com/art-framework-suite/cetlib/archive/refs/tags/v3_13_04.tar.gz'

    version('3.13.04', tag='v3_13_04', git=git_base, get_full_repo=True)
    version('3.15.00', tag='v3_15_00', git=git_base, get_full_repo=True)
    version('3.18.01', tag='v3_18_01', git=git_base, get_full_repo=True)

    variant('lite', default=True, when="@3.18.01:")
    variant('lite', default=False, when="@:3.15.00")

    patch('cetlib-notests.patch', when='@develop')
    patch('cetlib-3.15.00.patch', when='@3.15.00')
    patch('cetlib_openssl_spack.patch', when='@:3.15.00')
    patch('cetlib_CMakeLists.txt.patch', when='@3.18.01:')

    variant('cxxstd',
            default='17',
            values=('14', '17'),
            multi=False,
            description='Use the specified C++ standard when building.')

    depends_on('cmake', type='build')
    depends_on('cetmodules', type='build')
    depends_on('catch2', type=('build', 'link'))
    depends_on('intel-tbb', when='~lite', type=('build', 'link'))
    depends_on('sqlite', when='~lite', type=('build', 'link'))
    depends_on('openssl', when='~lite')
    depends_on('perl', when='~lite')  # Module skeletons, etc.  

    for build_type in ["Debug", "Release", "RelWithDebInfo"]:
        depends_on(f'cetlib-except build_type={build_type}', when=f'build_type={build_type}')

        depends_on(f'hep-concurrency build_type={build_type}', when=f'~lite build_type={build_type}')

        if build_type != "Debug":
           depends_on('boost', when=f'~lite build_type={build_type}') 
        else:
           depends_on('boost+debug', when='~lite build_type=Debug')

    if 'SPACKDEV_GENERATOR' in os.environ:
        generator = os.environ['SPACKDEV_GENERATOR']
        if generator.endswith('Ninja'):
            depends_on('ninja', type='build')

    def cmake_args(self):
        spec = self.spec
        cmake_args = [
            '-DCMAKE_INSTALL_PREFIX:PATH={0}'.format(spec.prefix),
            '-Dart_MODULE_PLUGINS=FALSE',
            '-DIGNORE_ABSOLUTE_TRANSITIVE_DEPENDENCIES=TRUE',
            '-DCMAKE_CXX_STANDARD={0}'.format(self.spec.variants['cxxstd'].value),
            '-DCETLIB_LITE={0}'.format(self.define_from_variant("CETLIB_LITE", "lite"))
        ]
        return cmake_args

    def setup_environment(self, spack_env, run_env):
        # Binaries.
        spack_env.prepend_path('PATH', os.path.join(self.build_directory, 'bin'))
        # For plugin tests (not needed for installed package).
        spack_env.prepend_path('CET_PLUGIN_PATH',
                               os.path.join(self.build_directory, 'lib'))
        # Perl modules.
        spack_env.prepend_path('PERL5LIB',
                               os.path.join(self.build_directory, 'perllib'))
        run_env.prepend_path('PERL5LIB', os.path.join(self.prefix, 'perllib'))
        run_env.set('CETLIB_DIR', self.prefix)
        spack_env.set('CETLIB_DIR', self.prefix)
        # Cleanup.
        sanitize_environments(spack_env, run_env)

    def setup_dependent_environment(self, spack_env, run_env, dependent_spec):
        # Binaries.
        spack_env.prepend_path('PATH', self.prefix.bin)
        run_env.prepend_path('PATH', self.prefix.bin)
        # Perl modules.
        spack_env.prepend_path('PERL5LIB', os.path.join(self.prefix, 'perllib'))
        run_env.prepend_path('PERL5LIB', os.path.join(self.prefix, 'perllib'))
        run_env.set('CETLIB_DIR', self.prefix)
        spack_env.set('CETLIB_DIR', self.prefix)

    @run_after('install')
    def rename_README(self):
        import os
        os.rename( join_path(self.spec.prefix, "README"),
                   join_path(self.spec.prefix, "README_%s"%self.spec.name))
