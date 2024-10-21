# Copyright 2013-2019 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class Cetmodules(CMakePackage):
    """CMake glue modules and scripts required by packages originating at
    Fermilab and associated experiments and other collaborations.
    """


    homepage = 'https://github.com/FNALssi/cetmodules'
    url = 'https://github.com/FNALssi/cetmodules/archive/refs/tags/3.18.00.tar.gz'

    version('2.25.05', sha256='62a97e99233754ef4a6754d3a467dd2ee40f8cc993905d257e73f0abb0457bc2')
    version('3.18.00', sha256='160ae30a80fa6633d0bcc5d05c5f89cf46432231be32b92753e09d5702cf5576')

    #depends_on('py-sphinx', type='build')
    #depends_on('py-sphinxcontrib-moderncmakedomain', type='build')
    #depends_on('py-sphinx-rtd-theme', type='build')
    depends_on('catch2@2.13.10', type='build')

    @run_before('cmake')
    def fix_fix_man(self):
        filter_file('exit \$status', 'exit 0', '%s/libexec/fix-man-dirs' % self.stage.source_path)

    def cmake_args(self):

        spec = self.spec

        cmake_args = [
            #'-DCMAKE_INSTALL_PREFIX:PATH={0}'.format(spec.prefix)
            '-DCMAKE_INSTALL_PREFIX:PATH={0}'.format(spec.prefix),
            self.define('BUILD_TESTING', False),
            self.define('BUILD_DOCS', False),
        ]

        return cmake_args


#    def url_for_version(self, version):
#         if str(version)[0] in "01"    

#    def url_for_version(self, version):
#        url = 'http://cdcvs.fnal.gov/cgi-bin/git_archive.cgi/cvs/projects/{0}.v{1}.tbz2'
#        return url.format(self.name, version.underscored)
